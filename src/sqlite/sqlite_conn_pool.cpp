#include <cassert>
#include <sqlite3.h>
#include <fmt/core.h>
#include "sqlite/sqlite_conn.h"
#include "sqlite/sqlite_conn_pool.h"

const size_t sqlite_conn_pool::defaultPoolSize = 15;
const size_t sqlite_conn_pool::defaultOptimizationTimeout = 10;

//static
//void sqlite3_hook(void * /*object*/, int op, const char * /*database*/, const char * /*table*/, sqlite_int64 /*key*/)
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
//}

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

sqlite_conn_pool::sqlite_conn_pool(_params params)
		:db_conn_pool(params.poolSize), mFilename(params.filename)
{
	initialize();
	start_optimization_thread(params.optimizationTimeout, 400);
}

sqlite_conn_pool::sqlite_conn_pool(std::string_view filename, const size_t poolSize, const size_t optimizationTimeout)
		:db_conn_pool(poolSize), mFilename(filename)
{
	initialize();
	start_optimization_thread(optimizationTimeout, 400);
}

sqlite_conn_pool::~sqlite_conn_pool()
{
    if (mDb) {
        stop_optimization_thread();
        commit();

        sqlite3_close(mDb);
    }
}

void sqlite_conn_pool::initialize()
{
	sqlite3_enable_shared_cache(1);

	// open handle to database for connection pool stuff
	if (SQLITE_OK != sqlite3_open(mFilename.c_str(), &mDb)) {
		throw std::runtime_error(fmt::format("Unable to open file '{}'", mFilename));
	}
}

std::shared_ptr<db_conn_pool> sqlite_conn_pool::create(std::string_view filename, const size_t poolSize,
		const size_t optimizationTimeout)
{
	_params params;
	params.filename = filename;
	params.poolSize = poolSize;
	params.optimizationTimeout = optimizationTimeout;

	auto pool = std::make_shared<sqlite_conn_pool>(params);
	return std::dynamic_pointer_cast<db_conn_pool>(pool);
}

std::shared_ptr<db_conn_pool> sqlite_conn_pool::shared_base_ptr()
{
	return std::dynamic_pointer_cast<db_conn_pool>(shared_from_this());
}

int64_t sqlite_conn_pool::get_schema()
{
	assert(mDb);

	// compile prepared statement
	std::string_view sql{"PRAGMA user_version"};
	sqlite3_stmt* stmt{};
	if (sqlite3_prepare_v2(mDb, sql.data(), sql.length() + 1, &stmt, nullptr)) {
		const auto err = fmt::format("sqlite_conn_pool::get_schema: {}", sqlite3_errmsg(mDb));
		throw std::runtime_error(err);
	}

		// execute the prepared statement
#ifdef NDEBUG
		sqlite3_step(stmt);
#else
	assert(SQLITE_ROW == sqlite3_step(stmt));
#endif
	const auto schema = sqlite3_column_int64(stmt, 0);
	sqlite3_finalize(stmt);

	return schema;
}

void sqlite_conn_pool::set_schema(const int64_t schema)
{
	assert(mDb);
	const auto sqlStmt = fmt::format("PRAGMA user_version = {}", schema);
	if (sqlite3_exec(mDb, sqlStmt.data(), nullptr, nullptr, nullptr)) {
		throw std::runtime_error(fmt::format("sqlite_conn_pool::set_schema: {}", sqlite3_errmsg(mDb)));
	}
}

db_conn* sqlite_conn_pool::new_conn()
{
	sqlite3* db{};
	if (sqlite3_open(mFilename.data(), &db)) {
		throw std::runtime_error(fmt::format("sqlite_conn_pool::new_conn: Unable to open file '{}'", mFilename));
	}
	return new sqlite_conn(db);
}

void sqlite_conn_pool::optimization_thread(const size_t timeout, const size_t threshold)
{
	assert(timeout > 0);

	sqlite3_stmt* stmt{};
	const auto sql = fmt::format("PRAGMA analysis_limit = {}; PRAGMA optimize;", threshold);
	if (sqlite3_prepare_v2(mDb, sql.data(), sql.length() + 1, &stmt, nullptr)) {
		const auto err = fmt::format("sqlite_conn_pool::optimization_thread: {}", sqlite3_errmsg(mDb));
		throw std::runtime_error(err);
	}

	auto cond = [this]() -> bool { return !mRunOptimizationThread; };

	do {
		std::unique_lock<std::mutex> lk(mOptimizationMutex);
		mOptimizationCondition.wait_for(lk, std::chrono::minutes(timeout), cond);
		sqlite3_step(stmt);
		sqlite3_reset(stmt);
	}
	while (mRunOptimizationThread);

	sqlite3_finalize(stmt);
}

void sqlite_conn_pool::start_optimization_thread(const size_t timeout, const size_t threshold)
{
	assert(!mOptimizationThread);
	if (timeout > 0) {
		mRunOptimizationThread = true;
		mOptimizationThread = std::make_unique<std::thread>(&sqlite_conn_pool::optimization_thread, this, timeout,
                threshold);
		mOptimizationCondition.notify_one();
	}
}

void sqlite_conn_pool::stop_optimization_thread()
{
	// optimize database one last time before exiting
	if (mOptimizationThread) {
		mRunOptimizationThread = false;
		mOptimizationCondition.notify_one();
		mOptimizationThread->join();
		mOptimizationThread.reset();
	}
}

void sqlite_conn_pool::commit()
{
	assert(mDb);
	if (sqlite3_exec(mDb, "VACUUM", nullptr, nullptr, nullptr)) {
		throw std::runtime_error(fmt::format("sqlite_conn_pool::commit: {}", sqlite3_errmsg(mDb)));
	}
}

std::string_view sqlite_conn_pool::get_filename() const
{
	return mFilename;
}

bool sqlite_conn_pool::is_open() const
{
	return mDb != nullptr;
}
