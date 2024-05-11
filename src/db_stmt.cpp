#include "db_stmt.h"
#include "db_conn.h"

//!
//! \brief Creates a prepared statement connected to the connect \c conn.
//!
//! \param conn Connection guard that created this prepared statement.
//!
db_stmt::db_stmt(std::shared_ptr<db_conn> conn) noexcept
    : mConn(conn)
{
}
