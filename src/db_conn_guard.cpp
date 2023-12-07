#include <cassert>
#include "db_conn.h"
#include "db_conn_guard.h"
#include "db_conn_pool.h"

//!
//! \brief Takes ownership of the members from \c connGuard.
//!
//! \param connGuard Connection guard to take ownership of.
//!
db_conn_guard::db_conn_guard(db_conn_guard&& connGuard)
{
	assert(connGuard.mConn);
	mConn = connGuard.mConn;
	connGuard.mConn = nullptr;

	assert(connGuard.mConnPool);
	mConnPool = std::move(connGuard.mConnPool);
}

//!
//! \brief Construct a connection guard with \c conn database connection and \c connPool database connection pool,
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

db_conn_guard::~db_conn_guard()
{
	if (mConn) {
		mConnPool->push_conn(mConn);
	}
}

db_conn_guard& db_conn_guard::operator=(db_conn_guard&& connGuard)
{
    // make sure old connection pool doesn't "leak" connections
    if (mConn && mConnPool) {
        mConnPool->push_conn(mConn);
    }

	assert(connGuard.mConn);
	mConn = connGuard.mConn;
	connGuard.mConn = nullptr;

	assert(connGuard.mConnPool);
	mConnPool = std::move(connGuard.mConnPool);

	return *this;
}

db_stmt::return_code db_conn_guard::exec(std::string_view sql)
{
	assert(mConn);
	return mConn->exec(sql);
}

std::unique_ptr<db_stmt> db_conn_guard::get_stmt(const std::string& sql)
{
	assert(mConn);
	return mConn->get_stmt(sql);
}
