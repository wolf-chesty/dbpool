#include <algorithm>
#include <cassert>
#include "db_conn.h"
#include "db_conn_pool.h"
#include "db_stmt.h"

//!
//! \brief Creates a connection pull with \c poolSize database connections.
//!
//! \param poolSize Size of the connection pool.
//!
db_conn_pool::db_conn_pool(const size_t poolSize)
{
	mAvailableConnections.resize(poolSize, nullptr);
}

//!
//! \brief Closes the connection pool.
//!
//! The \c db_conn_pool destructor will iterate through all of the open database connections and delete them.
//!
db_conn_pool::~db_conn_pool()
{
	// delete db connections
	std::unique_lock<std::mutex> lk(mConnectionMutex);
	std::for_each(mAvailableConnections.begin(), mAvailableConnections.end(), [](db_conn* conn) { delete conn; });
	lk.unlock();

	mConnectionCondition.notify_all();
}

//!
//! Returns a new \c db_conn_guard object that can be used to execute statements against the database.
//!
//! \return A new \c db_conn_guard.
//!
db_conn_guard db_conn_pool::get_conn()
{
	auto conn = pop_conn();
	return db_conn_guard(conn, shared_base_ptr());
}

//!
//! Returns an open \c db_conn to use with a database.
//!
//! \return An open \c db_conn.
//!
db_conn* db_conn_pool::pop_conn()
{
	// if there are available connections then remove the connection from the pool and return it; otherwise wait for a
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

	return conn;
}

//!
//! \brief Returns \c conn to the connection pool for reuse by other threads.
//!
//! \param conn Database connection to return to the connection pool.
//!
void db_conn_pool::push_conn(db_conn* conn)
{
	// add database connection back to pool
    std::unique_lock<std::mutex> lk(mConnectionMutex);
	mAvailableConnections.push_front(conn);
	lk.unlock();

    // notify threads waiting in pop_conn for a connection that a connection has been returned to the pool
	mConnectionCondition.notify_all();
}

//!
//! \brief Stores the SQL statement, \c sql, for use when preparing new database connections.
//!
//! \param sql SQL statement to execute when opening a new database connection.
//!
//! Some databases (such as SQLite3) require some statements (i.e., statements to create temporary table views) to be
//! executed on each database connection that is opened in order for each database connection to have a consistent view
//! of the database. This function sets the statement that you want to have executed on any newly created database
//! connections that get opened by this connection pool.
//!
void db_conn_pool::set_prep_sql(std::string_view sql)
{
	mPrepSql = sql;
}

//!
//! \brief Executes an SQL preparation statement against the database connection \c conn.
//!
//! \param conn Database connection to execute the preparation statement against.
//!
//! This function executes the preparatory statement against the \c conn database connection. For more information
//! about this see \c db_conn_pool::set_prep_sql.
//!
void db_conn_pool::prep_conn(db_conn* conn)
{
	if (!mPrepSql.empty())
		conn->exec(mPrepSql);
}
