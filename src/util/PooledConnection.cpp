// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include "dbpool/util/PooledConnection.hpp"

#include "dbpool/impl/ConnectionImpl.hpp"
#include "dbpool/impl/ConnectionPoolImpl.hpp"
#include <cassert>

using namespace dbpool;

PooledConnection::PooledConnection(std::shared_ptr<ConnectionPoolImpl> conn_pool, std::unique_ptr<ConnectionImpl> conn)
    : conn_pool_(std::move(conn_pool))
    , conn_(std::move(conn))
{
    assert(conn_pool_);
    assert(conn_);
}

PooledConnection::~PooledConnection()
{
    assert(conn_pool_);
    assert(conn_);
    conn_pool_->push_conn(std::move(conn_));
}

ConnectionImpl *PooledConnection::get_conn()
{
    assert(conn_);
    return conn_.get();
}
