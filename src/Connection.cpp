// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include "dbpool/Connection.hpp"

#include "dbpool/impl/ConnectionImpl.hpp"
#include "dbpool/impl/ConnectionPoolImpl.hpp"
#include "dbpool/impl/PreparedStmtImpl.hpp"
#include "dbpool/util/PooledConnection.hpp"
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
    auto &conn = pooled_conn_->getConnection();
    return conn.exec(sql);
}

PreparedStmt Connection::getStmt(std::string const &sql)
{
    assert(pooled_conn_);
    auto &conn = pooled_conn_->getConnection();
    return PreparedStmt(pooled_conn_, conn.getStmt(sql));
}