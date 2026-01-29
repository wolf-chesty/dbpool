#include "dbpool/Connection.hpp"

#include "dbpool/ConnectionImpl.hpp"
#include "dbpool/ConnectionPool.hpp"
#include <cassert>

using namespace dbpool;

//!
//! \brief Construct a connection guard with \c conn database connection and \c connPool database
//! connection pool.
//!
//! \param conn Database connection.
//! \param conn_pool Connection pool that owns the database connection \c conn.
//!
Connection::Connection(std::unique_ptr<ConnectionImpl> conn, std::shared_ptr<ConnectionPool> conn_pool)
    : m_conn(std::move(conn))
    , m_conn_pool(conn_pool)
{
    assert(m_conn);
    assert(m_conn_pool);
}

//!
//! \brief Destroys this object, returning the database connection to its connection pool.
//!
Connection::~Connection()
{
    assert(m_conn);
    assert(m_conn_pool);
    m_conn_pool->push_conn(std::move(m_conn));
}

//!
//! \brief Executes an SQL statements on this database connection.
//!
//! \param sql SQL statement to execute.
//!
//! \return \c db_stmt::return_code.
//!
PreparedStmt::return_code Connection::exec(std::string_view sql)
{
    assert(m_conn);
    return m_conn->exec(sql);
}

//!
//! \brief Get a prepared statement for this database connection.
//!
//! \param sql SQL statement to create the prepared statement from.
//!
//! \return Pointer to a prepared statement object.
//!
std::unique_ptr<PreparedStmt> Connection::get_stmt(std::string const &sql)
{
    assert(m_conn);
    return m_conn->get_stmt(shared_from_this(), sql);
}
