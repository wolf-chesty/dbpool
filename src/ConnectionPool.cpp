#include "dbpool/ConnectionPool.hpp"

#include "dbpool/ConnectionImpl.hpp"
#include "dbpool/PreparedStmt.hpp"
#include <algorithm>
#include <cassert>

using namespace dbpool;

//!
//! \brief Creates a connection pool with \c poolSize database connections.
//!
//! \param poolSize Size of the connection pool.
//!
ConnectionPool::ConnectionPool(size_t poolSize)
    : m_available_conns(poolSize)
{
}

//!
//! Returns a new \c db_conn object that can be used to execute statements against the database.
//!
//! \return A new \c db_conn.
//!
std::shared_ptr<Connection> ConnectionPool::get_conn()
{
    return std::make_shared<Connection>(pop_conn(), shared_base_ptr());
}

//!
//! Returns an open \c db_conn to use with a database.
//!
//! \return An open \c db_conn.
//!
std::unique_ptr<ConnectionImpl> ConnectionPool::pop_conn()
{
    // If there are available connections then remove the connection from the pool and return it; otherwise wait for a
    // connection to become available
    std::unique_lock<std::mutex> lk(m_conn_mutex);
    m_conn_condition.wait(lk, [this]() -> bool { return !m_available_conns.empty(); });

    // Grab connection from available pool
    auto conn = std::move(m_available_conns.front());
    m_available_conns.pop_front();

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
void ConnectionPool::push_conn(std::unique_ptr<ConnectionImpl> conn)
{
    assert(conn);

    // add database connection back to pool
    std::unique_lock<std::mutex> lk(m_conn_mutex);
    m_available_conns.emplace_front(std::move(conn));
    lk.unlock();

    // notify threads that a connection has been returned to the pool
    m_conn_condition.notify_all();
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
void ConnectionPool::set_prep_sql(std::string_view sql)
{
    m_prep_sql = sql;
}

//!
//! \brief Executes an SQL preparation statement against the database connection \c conn.
//!
//! \param conn Database connection to execute the preparation statement against.
//!
//! This function executes the preparatory statement against the \c conn database connection.
//!
void ConnectionPool::prep_conn(ConnectionImpl &conn)
{
    if (!m_prep_sql.empty()) {
        conn.exec(m_prep_sql);
    }
}
