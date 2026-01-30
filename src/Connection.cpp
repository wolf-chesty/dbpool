// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include "dbpool/Connection.hpp"

#include "dbpool/ConnectionImpl.hpp"
#include "dbpool/ConnectionPool.hpp"
#include <cassert>

using namespace dbpool;

Connection::Connection(std::unique_ptr<ConnectionImpl> conn, std::shared_ptr<ConnectionPool> conn_pool)
    : conn_(std::move(conn))
    , conn_pool_(std::move(conn_pool))
{
    assert(conn_);
    assert(conn_pool_);
}

Connection::~Connection()
{
    assert(conn_);
    assert(conn_pool_);
    conn_pool_->push_conn(std::move(conn_));
}

PreparedStmt::return_code Connection::exec(std::string_view sql)
{
    assert(conn_);
    return conn_->exec(sql);
}

std::unique_ptr<PreparedStmt> Connection::get_stmt(std::string const &sql)
{
    assert(conn_);
    return conn_->get_stmt(shared_from_this(), sql);
}
