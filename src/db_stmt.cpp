#include "dbpool/db_stmt.h"
#include "dbpool/db_conn.h"

using namespace dbpool;

//!
//! \brief Creates a prepared statement connected to the connect \c conn.
//!
//! \param conn Connection guard that created this prepared statement.
//!
db_stmt::db_stmt(std::shared_ptr<db_conn> conn) noexcept
    : m_conn(conn)
{
}

//!
//! \brief Returns the connection that created this statement.
//!
//! \return Connection that created this statement.
//!
std::shared_ptr<db_conn> db_stmt::get_conn()
{
    return m_conn;
}
