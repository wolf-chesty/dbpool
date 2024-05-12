#include "db_conn_pool.h"

#include <algorithm>
#include <cassert>

#include "db_conn_impl.h"
#include "db_stmt.h"

using namespace dbpool;

//!
//! \brief Creates a connection pool with \c poolSize database connections.
//!
//! \param poolSize Size of the connection pool.
//!
db_conn_pool::db_conn_pool(size_t const poolSize)
    : mAvailableConnections(poolSize)
{
}

//!
//! Returns a new \c db_conn object that can be used to execute statements against the database.
//!
//! \return A new \c db_conn.
//!
std::shared_ptr<db_conn> db_conn_pool::get_conn()
{
    return std::make_shared<db_conn>(pop_conn(), shared_base_ptr());
}

//!
//! Returns an open \c db_conn to use with a database.
//!
//! \return An open \c db_conn.
//!
std::unique_ptr<db_conn_impl> db_conn_pool::pop_conn()
{
    // If there are available connections then remove the connection from the pool and return it; otherwise wait for a
    // connection to become available
    std::unique_lock<std::mutex> lk(mConnectionMutex);
    mConnectionCondition.wait(lk, [this]() -> bool { return !mAvailableConnections.empty(); });

    // Grab connection from available pool
    auto conn = std::move(mAvailableConnections.front());
    mAvailableConnections.pop_front();

    // Open new database connection
    if (!conn) {
        conn = new_conn();
        prep_conn(*conn);
    }

    return conn;
}

//!
//! \brief Returns \c conn to the connection pool for reuse by other threads.
//!
//! \param conn Database connection to return to the connection pool.
//!
void db_conn_pool::push_conn(std::unique_ptr<db_conn_impl> conn)
{
    assert(conn);

    // add database connection back to pool
    std::unique_lock<std::mutex> lk(mConnectionMutex);
    mAvailableConnections.emplace_front(std::move(conn));
    lk.unlock();

    // notify threads that a connection has been returned to the pool
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
//! This function executes the preparatory statement against the \c conn database connection.
//!
void db_conn_pool::prep_conn(db_conn_impl &conn)
{
    if (!mPrepSql.empty())
        conn.exec(mPrepSql);
}
