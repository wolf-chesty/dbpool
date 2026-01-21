#include "dbpool/db_conn.h"

#include <cassert>

#include "dbpool/db_conn_impl.h"
#include "dbpool/db_conn_pool.h"

using namespace dbpool;

//!
//! \brief Construct a connection guard with \c conn database connection and \c connPool database
//! connection pool.
//!
//! \param conn Database connection.
//! \param conn_pool Connection pool that owns the database connection \c conn.
//!
db_conn::db_conn(std::unique_ptr<db_conn_impl> conn, std::shared_ptr<db_conn_pool> conn_pool)
    : m_conn(std::move(conn))
    , m_conn_pool(conn_pool)
{
    assert(m_conn);
    assert(m_conn_pool);
}

//!
//! \brief Destroys this object, returning the database connection to its connection pool.
//!
db_conn::~db_conn()
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
db_stmt::return_code db_conn::exec(std::string_view sql)
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
std::unique_ptr<db_stmt> db_conn::get_stmt(std::string const &sql)
{
    assert(m_conn);
    return m_conn->get_stmt(shared_from_this(), sql);
}
