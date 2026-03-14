// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include "dbpool/impl/ConnectionPoolImpl.hpp"

#include "dbpool/impl/ConnectionImpl.hpp"
#include <cassert>
#include <iterator>

using namespace dbpool;

ConnectionPoolImpl::ConnectionPoolImpl(size_t pool_size)
	: available_conns_(pool_size)
{
}

std::unique_ptr<ConnectionImpl> ConnectionPoolImpl::popConnection()
{
	// If there are available connections then remove the connection from the pool and return it; otherwise wait for a
	// connection to become available
	std::unique_lock<std::mutex> lk(conn_mutex_);
	conn_cv_.wait(lk, [this]() -> bool { return !available_conns_.empty(); });

	// Grab connection from available pool
	auto conn = std::move(available_conns_.front());
	available_conns_.pop_front();

	// Open new database connection
	if (!conn) {
		conn = createConnection();
		prepareConnection(*conn);
	}

	return conn;
}

void ConnectionPoolImpl::pushConnection(std::unique_ptr<ConnectionImpl> conn)
{
	assert(conn);

	// add database connection back to pool
	std::unique_lock<std::mutex> lk(conn_mutex_);
	available_conns_.emplace_front(std::move(conn));
	lk.unlock();

	// notify threads that a connection has been returned to the pool
	conn_cv_.notify_all();
}

size_t ConnectionPoolImpl::count()
{
	std::unique_lock<std::mutex> lk(conn_mutex_);
	return std::distance(available_conns_.begin(), available_conns_.end());
}

void ConnectionPoolImpl::setPrepSql(std::string_view sql)
{
	prep_sql_ = sql;
}

void ConnectionPoolImpl::prepareConnection(ConnectionImpl& conn) const
{
	if (!prep_sql_.empty()) {
		conn.exec(prep_sql_);
	}
}