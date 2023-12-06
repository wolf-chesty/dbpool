#include <algorithm>
#include <cassert>
#include "db_conn.h"
#include "db_conn_pool.h"
#include "db_stmt.h"

db_conn_pool::db_conn_pool(const size_t poolSize)
{
	mAvailableConnections.resize(poolSize, nullptr);
}

//!
//! \brief Closes the connection pool
//!
//! The \c db_conn_pool destructor will iterate through all of the open database connections and delete them causing the
//! database connections to close.
//!
//! Any threads that are using this pool really need to be done at this point. If the connection pool is changed in the
//! middle of a threads execution then results may not be what is expected, especially if results are compared between
//! to different statements. Measures have been taken to ensure that the application not crash when the pool is deleted
//! (i.e., statements will keep the connection pool open for the duration of their lifetime) but some work will need to
//! be done to ensure that each thread is using the same connection pool for their lifetime.
//!
db_conn_pool::~db_conn_pool()
{
	assert(mInUseConnections.empty());

	// close db connections
	std::unique_lock<std::mutex> lk(mConnectionMutex);
	std::for_each(mAvailableConnections.begin(), mAvailableConnections.end(), [](db_conn* conn) { delete conn; });
	lk.unlock();

	mConnectionCondition.notify_all();
}

db_conn_guard db_conn_pool::get_conn()
{
	auto conn = pop_conn();
	return db_conn_guard(conn, shared_base_ptr());
}

db_conn* db_conn_pool::pop_conn()
{
	// if there are available connections then move the connection to a used state and return it; otherwise wait for a
	// connection to become available
	std::unique_lock<std::mutex> lk(mConnectionMutex);
	mConnectionCondition.wait(lk, [this]() -> bool { return !mAvailableConnections.empty(); });

	// grab connection from available pool
	auto conn = mAvailableConnections.front();
	mAvailableConnections.pop_front();

	// open new database connection if it's not already open
	if (!conn) {
		conn = new_conn();
		prep_conn(conn);
	}

	// place connection in used queue
	mInUseConnections.push_front(conn);

	return conn;
}

void db_conn_pool::push_conn(db_conn* conn)
{
	// add database connection back to pool
	std::unique_lock<std::mutex> lk(mConnectionMutex);
	mInUseConnections.remove(conn);
	mAvailableConnections.push_front(conn);
	lk.unlock();

	mConnectionCondition.notify_all();
}

void db_conn_pool::set_prep_sql(std::string_view sql)
{
	mPrepSql = sql;
}

void db_conn_pool::prep_conn(db_conn* conn)
{
	if (!mPrepSql.empty())
		conn->exec(mPrepSql);
}
