#include "db_stmt.h"
#include "db_conn_guard.h"

//!
//! \brief Creates a prepared statement connected to the connect \c conn.
//!
//! \param conn Connection guard that created this prepared statement.
//!
db_stmt::db_stmt(std::shared_ptr<db_conn_guard> conn)
		:mConn(conn)
{
}

//!
//! \brief Takes ownership from of the prepared statement \c stmt.
//!
//! \param stmt Takes ownership of the prepared statement \c stmt.
//!
db_stmt::db_stmt(db_stmt&& stmt)
{
	mConn= std::move(stmt.mConn);
}
