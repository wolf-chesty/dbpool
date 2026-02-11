// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include "dbpool/sqlite/ConnectionImpl.hpp"

#include "dbpool/sqlite/PreparedStmtImpl.hpp"
#include <cassert>
#include <fmt/core.h>

using namespace dbpool::sqlite;

ConnectionImpl::ConnectionImpl(sqlite3 *db)
    : db_(db)
{
    assert(db_);
}

ConnectionImpl::~ConnectionImpl()
{
    assert(db_);
    sqlite3_close(db_);
}

PreparedStmtImpl::ReturnCode ConnectionImpl::exec(std::string_view sql)
{
    assert(db_);
    return PreparedStmtImpl::to_error_code(sqlite3_exec(db_, sql.data(), nullptr, nullptr, nullptr));
}

std::unique_ptr<dbpool::PreparedStmtImpl> ConnectionImpl::get_stmt(std::string const &sql)
{
    assert(db_);
    return std::make_unique<PreparedStmtImpl>(db_, sql);
}
