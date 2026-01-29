#include "dbpool/PreparedStmt.hpp"
#include "dbpool/Connection.hpp"

using namespace dbpool;

//!
//! \brief Creates a prepared statement connected to the connect \c conn.
//!
//! \param conn Connection guard that created this prepared statement.
//!
PreparedStmt::PreparedStmt(std::shared_ptr<Connection> conn) noexcept
    : m_conn(conn)
{
}

//!
//! \brief Returns the connection that created this statement.
//!
//! \return Connection that created this statement.
//!
std::shared_ptr<Connection> PreparedStmt::get_conn()
{
    return m_conn;
}
