#include <cassert>
#include "db_conn.h"
#include "db_conn_guard.h"
#include "db_conn_pool.h"

//!
//! \brief Construct a connection guard with \c conn database connection and \c connPool database connection pool.
//!
//! \param conn Database connection.
//! \param connPool Connection pool that owns the database connection \c conn.
//!
db_conn_guard::db_conn_guard(db_conn* conn, std::shared_ptr<db_conn_pool> connPool)
		:mConn(conn), mConnPool(connPool)
{
	assert(mConn);
	assert(mConnPool);
}

//!
//! \brief Destroys this object, returning the database connection to its connection pool.
//!
db_conn_guard::~db_conn_guard()
{
    assert(mConnPool);
    assert(mConn);

    if (mConn)
		mConnPool->push_conn(mConn);
}

//!
//! \brief Executes an SQL statements on this database connection.
//!
//! \param sql SQL statement to execute.
//! \return \c db_stmt::return_code.
//!
db_stmt::return_code db_conn_guard::exec(std::string_view sql)
{
	assert(mConn);
	return mConn->exec(sql);
}

//!
//! \brief Get a prepared statement for this database connection.
//!
//! \param sql SQL statement to create the prepared statement from.
//! \return Pointer to a prepared statement object.
//!
std::unique_ptr<db_stmt> db_conn_guard::get_stmt(const std::string& sql)
{
	assert(mConn);
	return mConn->get_stmt(shared_from_this(), sql);
}
