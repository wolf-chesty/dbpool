// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include "dbpool/Connection.hpp"

#include "dbpool/ConnectionImpl.hpp"
#include "dbpool/ConnectionPoolImpl.hpp"
#include "dbpool/PooledConnection.hpp"
#include "dbpool/PreparedStmtImpl.hpp"
#include <cassert>

using namespace dbpool;

Connection::Connection(std::shared_ptr<PooledConnection> pooled_conn)
    : pooled_conn_(std::move(pooled_conn))
{
    assert(pooled_conn_);
}

PreparedStmt::ReturnCode Connection::exec(std::string_view sql)
{
    assert(pooled_conn_);
    auto conn = pooled_conn_->get_conn();
    return conn->exec(sql);
}

PreparedStmt Connection::get_stmt(std::string const &sql)
{
    assert(pooled_conn_);
    auto conn = pooled_conn_->get_conn();
    return PreparedStmt(pooled_conn_, conn->get_stmt(sql));
}
