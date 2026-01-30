// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include "dbpool/PreparedStmt.hpp"

#include "dbpool/Connection.hpp"

using namespace dbpool;

PreparedStmt::PreparedStmt(std::shared_ptr<Connection> conn) noexcept
    : conn_(conn)
{
}

std::shared_ptr<Connection> PreparedStmt::get_conn()
{
    return conn_;
}
