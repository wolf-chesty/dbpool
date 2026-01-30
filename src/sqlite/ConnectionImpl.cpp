// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include "dbpool/sqlite/ConnectionImpl.hpp"

#include "dbpool/sqlite/PreparedStmt.hpp"
#include <cassert>
#include <fmt/core.h>

using namespace dbpool::sqlite;

ConnectionImpl::ConnectionImpl(sqlite3 *db)
    : db_(db)
{
}

ConnectionImpl::~ConnectionImpl()
{
    for (auto &stmt : stmt_cache_) {
        sqlite3_finalize(stmt.second);
    }

    assert(db_);
    sqlite3_close(db_);
}

dbpool::PreparedStmt::return_code ConnectionImpl::exec(std::string_view sql)
{
    assert(db_);
    return PreparedStmt::to_error_code(sqlite3_exec(db_, sql.data(), nullptr, nullptr, nullptr));
}

std::unique_ptr<dbpool::PreparedStmt> ConnectionImpl::get_stmt(std::shared_ptr<dbpool::Connection> conn,
                                                               std::string const &sql)
{
    assert(db_);

    if (auto it = stmt_cache_.find(sql); it != stmt_cache_.end()) {
        return std::make_unique<PreparedStmt>(conn, db_, it->second);
    }

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db_, sql.data(), static_cast<int>(sql.length() + 1), &stmt, nullptr)) {
        throw std::runtime_error(fmt::format("sqlite_conn_impl::get_stmt: {}", sqlite3_errmsg(db_)));
    }
    stmt_cache_.emplace(sql, stmt);

    return std::make_unique<PreparedStmt>(conn, db_, stmt);
}
