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
}

ConnectionImpl::~ConnectionImpl()
{
    for (auto const &stmt : stmt_cache_) {
        sqlite3_finalize(stmt.second);
    }

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

    if (auto const it = stmt_cache_.find(sql); it != stmt_cache_.end()) {
        return std::make_unique<PreparedStmtImpl>(db_, it->second);
    }

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.data(), static_cast<int>(sql.length() + 1), &stmt, nullptr)) {
        throw std::runtime_error(fmt::format("sqlite_conn_impl::get_stmt: {}", sqlite3_errmsg(db_)));
    }
    stmt_cache_.emplace(sql, stmt);

    return std::make_unique<PreparedStmtImpl>(db_, stmt);
}
