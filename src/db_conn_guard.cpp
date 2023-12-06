#include <cassert>
#include "db_conn.h"
#include "db_conn_guard.h"
#include "db_conn_pool.h"

db_conn_guard::db_conn_guard(db_conn_guard&& connGuard)
{
	assert(connGuard.mConn);
	mConn = connGuard.mConn;
	connGuard.mConn = nullptr;

	assert(connGuard.mConnPool);
	mConnPool = std::move(connGuard.mConnPool);
}

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
		mConn = nullptr;
	}
}

db_conn_guard& db_conn_guard::operator=(db_conn_guard&& connGuard)
{
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
