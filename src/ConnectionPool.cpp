// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include "dbpool/ConnectionPool.hpp"

#include "dbpool/ConnectionImpl.hpp"
#include "dbpool/PreparedStmt.hpp"
#include <algorithm>
#include <cassert>

using namespace dbpool;

ConnectionPool::ConnectionPool(size_t pool_size)
    : available_conns_(pool_size)
{
}

std::shared_ptr<Connection> ConnectionPool::get_conn()
{
    return std::make_shared<Connection>(pop_conn(), shared_base_ptr());
}

std::unique_ptr<ConnectionImpl> ConnectionPool::pop_conn()
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
        conn = new_conn();
        prep_conn(*conn);
    }

    return conn;
}

void ConnectionPool::push_conn(std::unique_ptr<ConnectionImpl> conn)
{
    assert(conn);

    // add database connection back to pool
    std::unique_lock<std::mutex> lk(conn_mutex_);
    available_conns_.emplace_front(std::move(conn));
    lk.unlock();

    // notify threads that a connection has been returned to the pool
    conn_cv_.notify_all();
}

void ConnectionPool::set_prep_sql(std::string_view sql)
{
    prep_sql_ = sql;
}

void ConnectionPool::prep_conn(ConnectionImpl &conn)
{
    if (!prep_sql_.empty()) {
        conn.exec(prep_sql_);
    }
}
