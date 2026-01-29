#include "dbpool/sqlite/ConnectionPool.hpp"

#include "dbpool/sqlite/ConnectionImpl.hpp"
#include <cassert>
#include <fmt/core.h>
#include <sqlite3.h>

using namespace dbpool::sqlite;

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
//! \param pool_size Number of connections in the database connection pool.
//! \param optimization_period Number of minutes to wait between database optimizations.
//!
//! This class will construct an \c sqlite_conn_pool object. This database will have \c filename as its filename and
//! contain \c poolSize database connections. Every \c optimizationTimeout minutes the connection pool will invoke a
//! function call that will optimize the record layout of the database file.
//!
//! An \c optimizationTimeout value <= 0 will cause this class to not start the optimization thread and optimization
//! will not be performed.
//!
ConnectionPool::ConnectionPool(std::string_view filename, size_t pool_size, optimization_period_t optimization_period)
    : dbpool::ConnectionPool(pool_size)
{
    // set SQLite3 to multi-threaded mode
    sqlite3_config(SQLITE_CONFIG_MULTITHREAD, nullptr);

    // open handle to database for connection pool stuff
    if (SQLITE_OK != sqlite3_open(filename.data(), &m_db)) {
        throw std::runtime_error(fmt::format("Unable to open file '{}'", filename));
    }

    start_optimization_thread(std::move(optimization_period), 400);
}

//!
//! \brief Destructs the connection pool.
//!
//! This destructor will destroy the connection, performing a final optimization of the file contents (final
//! optimization is skipped if the class was created with an optimization timeout value <= 0), defragments the file to
//! disk, and then closes the database file.
//!
ConnectionPool::~ConnectionPool()
{
    if (m_db) {
        stop_optimization_thread();
        commit();

        sqlite3_close(m_db);
    }
}

//!
//! \brief Creates and returns an \c sqlite_conn_pool connection pool object.
//!
//! \param filename Filename of the SQLite database.
//! \param pool_size Number of connections in the connection pool.
//! \param optimization_period Number of minutes to wait between optimization calls.
//!
//! \return Pointer to the connection pool.
//!
std::shared_ptr<dbpool::ConnectionPool> ConnectionPool::create(std::string_view filename, size_t pool_size,
                                                               optimization_period_t optimization_period)
{
    return std::make_shared<ConnectionPool>(filename, pool_size, optimization_period);
}

//!
//! \brief Returns a \c shared_ptr to the base class for this object.
//!
//! \return A \c shared_ptr to the base class for this object.
//!
std::shared_ptr<dbpool::ConnectionPool> ConnectionPool::shared_base_ptr()
{
    return std::dynamic_pointer_cast<dbpool::ConnectionPool>(shared_from_this());
}

//!
//! \brief Returns the schema number of the database.
//!
//! \return Schema number of the database.
//!
int64_t ConnectionPool::get_schema()
{
    assert(m_db);

    // Compile prepared statement
    std::string_view sql{"PRAGMA user_version"};
    sqlite3_stmt *stmt{};
    if (sqlite3_prepare_v2(m_db, sql.data(), sql.length() + 1, &stmt, nullptr)) {
        throw std::runtime_error(fmt::format("sqlite_conn_pool::get_schema: {}", sqlite3_errmsg(m_db)));
    }

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
void ConnectionPool::set_schema(int64_t schema)
{
    assert(m_db);
    auto const sqlStmt = fmt::format("PRAGMA user_version = {}", schema);
    if (sqlite3_exec(m_db, sqlStmt.data(), nullptr, nullptr, nullptr)) {
        throw std::runtime_error(fmt::format("sqlite_conn_pool::set_schema: {}", sqlite3_errmsg(m_db)));
    }
}

//!
//! \brief Creates and returns a pointer of a \c sqlite_conn_impl object for the managed database.
//!
//! \return \c sqlite_conn_impl class for the managed database.
//!
std::unique_ptr<dbpool::ConnectionImpl> ConnectionPool::new_conn()
{
    sqlite3 *db{};
    auto filename = get_filename();
    if (sqlite3_open(filename.c_str(), &db)) {
        throw std::runtime_error(fmt::format("sqlite_conn_pool::new_conn: Unable to open file '{}'", filename));
    }
    return std::make_unique<ConnectionImpl>(db);
}

//!
//! \brief Thread that periodically optimizes the database.
//!
//! \param period Number of minutes to wait between optimization calls.
//! \param threshold Number of records to examine when performing the optimization.
//!
//! This function will wake up periodically (every \c timeout minutes) and perform an optimization on the SQLite
//! database file.
//!
//! Time taken to optimize the database does not count towards the timeout length.
//!
void ConnectionPool::optimization_thread(sqlite3_stmt *stmt, optimization_period_t period,
                                         [[maybe_unused]] size_t threshold)
{
    assert(period.count() > 0);

    do {
        std::unique_lock<std::mutex> lk(m_optimization_mutex);
        m_optimization_condition.wait_for(lk, period, [this]() -> bool { return !m_run_optimization_thread; });

        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    } while (m_run_optimization_thread);

    sqlite3_finalize(stmt);
}

//!
//! \brief Starts the optimization thread.
//!
//! \param period Number of minutes to wait between calls to the optimization thread.
//! \param threshold Number of records to optimize at once.
//!
void ConnectionPool::start_optimization_thread(optimization_period_t period, size_t threshold)
{
    std::scoped_lock lk(m_optimization_thread_mutex);

    if (period.count() > 0 && !m_optimization_thread) {
        sqlite3_stmt *stmt{};
        auto const sql = fmt::format("PRAGMA analysis_limit = {}; PRAGMA optimize;", threshold);
        if (sqlite3_prepare_v2(m_db, sql.data(), sql.length() + 1, &stmt, nullptr)) {
            throw std::runtime_error(fmt::format("sqlite_conn_pool::optimization_thread: {}", sqlite3_errmsg(m_db)));
        }

        m_run_optimization_thread = true;
        m_optimization_thread = std::make_unique<std::thread>(&ConnectionPool::optimization_thread, this, stmt,
                                                              std::move(period), threshold);
    }
}

//!
//! \brief Stops the optimization thread.
//!
//! If the optimization thread was started for this connection pool then this function will stop
//! that thread. Otherwise, this function is effectively a no-op and does nothing.
//!
void ConnectionPool::stop_optimization_thread()
{
    // Optimize database one last time before exiting
    std::scoped_lock lk(m_optimization_thread_mutex);
    if (m_optimization_thread) {
        m_run_optimization_thread = false;
        m_optimization_condition.notify_one();

        m_optimization_thread->join();
        m_optimization_thread.reset();
    }
}

//!
//! \brief Defragments the database file, removing empty spaces caused by record deletions.
//!
void ConnectionPool::commit()
{
    assert(m_db);
    if (sqlite3_exec(m_db, "VACUUM", nullptr, nullptr, nullptr)) {
        throw std::runtime_error(fmt::format("sqlite_conn_pool::commit: {}", sqlite3_errmsg(m_db)));
    }
}

//!
//! \brief Returns the database filename.
//!
//! \return Filename of the database.
//!
std::string ConnectionPool::get_filename() const
{
    auto f = sqlite3_db_filename(m_db, nullptr);
    return std::string(!f || strlen(f) == 0 ? ":memory:" : f);
}

//!
//! \brief Returns true if the database was opened.
//!
//! \return \c true if the database is open.
//!
bool ConnectionPool::is_open() const
{
    return m_db != nullptr;
}
