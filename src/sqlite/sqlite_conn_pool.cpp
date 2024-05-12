#include "sqlite/sqlite_conn_pool.h"

#include <cassert>
#include <fmt/core.h>
#include <sqlite3.h>

#include "sqlite/sqlite_conn_impl.h"

using namespace dbpool;

size_t const sqlite_conn_pool::defaultPoolSize = 15;
size_t const sqlite_conn_pool::defaultOptimizationTimeout = 10;

// static
// void sqlite3_hook(void * /*object*/, int op, const char * /*database*/, const char * /*table*/,
// sqlite_int64 /*key*/)
//{
//	switch (op)
//	{
//	case SQLITE_INSERT:
//		break;
//
//	case SQLITE_DELETE:
//		break;
//
//	case SQLITE_UPDATE:
//		break;
//
//	default:
//		break;
//	}
// }

/*
static
int exec_callback(void *data, int c_num, char **c_vals, char **c_names)
{
        TRACE("%s:\n", static_cast<const char *>(data));
        for (decltype(c_num) i = 0; i < c_num; ++i)
        {
                TRACE("\t%s = %s\n", c_names + i, c_vals[i] ? c_vals[i] : "nullptr");
        }
        TRACE("\n");
        return 0;
}
*/

//!
//! \brief Constructs \c sqlite_conn_pool object.
//!
//! \param filename Database filename.
//! \param poolSize Number of connections in the database connection pool.
//! \param optimizationTimeout Number of minutes to wait between database optimizations.
//!
//! This class will construct an \c sqlite_conn_pool object. This database will have \c filename as its filename and
//! contain \c poolSize database connections. Every \c optimizationTimeout minutes the connection pool will invoke a
//! function call that will optimize the record layout of the database file.
//!
//! An \c optimizationTimeout value <= 0 will cause this class to not start the optimization thread and optimization
//! will not be performed.
//!
sqlite_conn_pool::sqlite_conn_pool(std::string_view filename, size_t const poolSize, size_t const optimizationTimeout)
    : db_conn_pool(poolSize)
{
    // set SQLite3 to multi-threaded mode
    sqlite3_config(SQLITE_CONFIG_MULTITHREAD, nullptr);

    // open handle to database for connection pool stuff
    if (SQLITE_OK != sqlite3_open(filename.data(), &mDb))
        throw std::runtime_error(fmt::format("Unable to open file '{}'", filename));

    start_optimization_thread(optimizationTimeout, 400);
}

//!
//! \brief Destructs the connection pool.
//!
//! This destructor will destroy the connection, performing a final optimization of the file contents (final
//! optimization is skipped if the class was created with an optimization timeout value <= 0), defragments the file to
//! disk, and then closes the database file.
//!
sqlite_conn_pool::~sqlite_conn_pool()
{
    if (mDb) {
        stop_optimization_thread();
        commit();

        sqlite3_close(mDb);
    }
}

//!
//! \brief Creates and returns an \c sqlite_conn_pool connection pool object.
//!
//! \param filename Filename of the SQLite database.
//! \param poolSize Number of connections in the connection pool.
//! \param optimizationTimeout Number of minutes to wait between optimization calls.
//!
//! \return Pointer to the connection pool.
//!
std::shared_ptr<db_conn_pool> sqlite_conn_pool::create(std::string_view filename, size_t const poolSize,
                                                       size_t const optimizationTimeout)
{
    std::shared_ptr<sqlite_conn_pool> pool(new sqlite_conn_pool(filename, poolSize, optimizationTimeout));
    return std::dynamic_pointer_cast<db_conn_pool>(pool);
}

//!
//! \brief Returns a \c shared_ptr to the base class for this object.
//!
//! \return A \c shared_ptr to the base class for this object.
//!
std::shared_ptr<db_conn_pool> sqlite_conn_pool::shared_base_ptr()
{
    return std::dynamic_pointer_cast<db_conn_pool>(shared_from_this());
}

//!
//! \brief Returns the schema number of the database.
//!
//! \return Schema number of the database.
//!
int64_t sqlite_conn_pool::get_schema()
{
    assert(mDb);

    // Compile prepared statement
    std::string_view sql{"PRAGMA user_version"};
    sqlite3_stmt *stmt{};
    if (sqlite3_prepare_v2(mDb, sql.data(), sql.length() + 1, &stmt, nullptr))
        throw std::runtime_error(fmt::format("sqlite_conn_pool::get_schema: {}", sqlite3_errmsg(mDb)));

        // Execute the prepared statement
#ifdef NDEBUG
    sqlite3_step(stmt);
#else
    assert(SQLITE_ROW == sqlite3_step(stmt));
#endif

    auto const schema = sqlite3_column_int64(stmt, 0);
    sqlite3_finalize(stmt);

    return schema;
}
//!
//! \brief Sets the schema number of the database.
//!
//! \param schema Schema number of the database.
//!
void sqlite_conn_pool::set_schema(int64_t const schema)
{
    assert(mDb);
    auto const sqlStmt = fmt::format("PRAGMA user_version = {}", schema);
    if (sqlite3_exec(mDb, sqlStmt.data(), nullptr, nullptr, nullptr))
        throw std::runtime_error(fmt::format("sqlite_conn_pool::set_schema: {}", sqlite3_errmsg(mDb)));
}

//!
//! \brief Creates and returns a pointer of a \c sqlite_conn_impl object for the managed database.
//!
//! \return \c sqlite_conn_impl class for the managed database.
//!
std::unique_ptr<db_conn_impl> sqlite_conn_pool::new_conn()
{
    sqlite3 *db{};
    auto filename = get_filename();
    if (sqlite3_open(filename.c_str(), &db))
        throw std::runtime_error(fmt::format("sqlite_conn_pool::new_conn: Unable to open file '{}'", filename));
    return std::make_unique<sqlite_conn_impl>(db);
}

//!
//! \brief Thread that periodically optimizes the database.
//!
//! \param timeout Number of minutes to wait between optimization calls.
//! \param threshold Number of records to examine when performing the optimization.
//!
//! This function will wake up periodically (every \c timeout minutes) and perform an optimization on the SQLite
//! database file.
//!
//! Time taken to optimize the database does not count towards the timeout length.
//!
void sqlite_conn_pool::optimization_thread(sqlite3_stmt *stmt, size_t const timeout, size_t const threshold)
{
    assert(timeout > 0);

    auto cond = [this]() -> bool { return !mRunOptimizationThread; };

    do {
        std::unique_lock<std::mutex> lk(mOptimizationMutex);
        mOptimizationCondition.wait_for(lk, std::chrono::minutes(timeout), cond);

        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    } while (mRunOptimizationThread);

    sqlite3_finalize(stmt);
}

//!
//! \brief Starts the optimization thread.
//!
//! \param timeout Number of minutes to wait between calls to the optimization thread.
//! \param threshold Number of records to optimize at once.
//!
void sqlite_conn_pool::start_optimization_thread(size_t const timeout, size_t const threshold)
{
    std::scoped_lock lk(mOptimizationThreadMutex);

    if (timeout > 0 && !mOptimizationThread) {
        sqlite3_stmt *stmt{};
        auto const sql = fmt::format("PRAGMA analysis_limit = {}; PRAGMA optimize;", threshold);
        if (sqlite3_prepare_v2(mDb, sql.data(), sql.length() + 1, &stmt, nullptr))
            throw std::runtime_error(fmt::format("sqlite_conn_pool::optimization_thread: {}", sqlite3_errmsg(mDb)));

        mRunOptimizationThread = true;
        mOptimizationThread =
            std::make_unique<std::thread>(&sqlite_conn_pool::optimization_thread, this, stmt, timeout, threshold);
    }
}

//!
//! \brief Stops the optimization thread.
//!
//! If the optimization thread was started for this connection pool then this function will stop
//! that thread. Otherwise, this function is effectively a no-op and does nothing.
//!
void sqlite_conn_pool::stop_optimization_thread()
{
    // Optimize database one last time before exiting
    std::scoped_lock lk(mOptimizationThreadMutex);
    if (mOptimizationThread) {
        mRunOptimizationThread = false;
        mOptimizationCondition.notify_one();

        mOptimizationThread->join();
        mOptimizationThread.reset();
    }
}

//!
//! \brief Defragments the database file, removing empty spaces caused by record deletions.
//!
void sqlite_conn_pool::commit()
{
    assert(mDb);
    if (sqlite3_exec(mDb, "VACUUM", nullptr, nullptr, nullptr))
        throw std::runtime_error(fmt::format("sqlite_conn_pool::commit: {}", sqlite3_errmsg(mDb)));
}

//!
//! \brief Returns the database filename.
//!
//! \return Filename of the database.
//!
std::string sqlite_conn_pool::get_filename() const
{
    auto f = sqlite3_db_filename(mDb, nullptr);
    return std::string(!f || strlen(f) == 0 ? ":memory:" : f);
}

//!
//! \brief Returns true if the database was opened.
//!
//! \return \c true if the database is open.
//!
bool sqlite_conn_pool::is_open() const
{
    return mDb != nullptr;
}
