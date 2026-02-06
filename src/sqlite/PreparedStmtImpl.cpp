// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include "dbpool/sqlite/PreparedStmtImpl.hpp"

#include <cassert>
#include <chrono>
#include <cstring>
#include <date/date.h>
#include <fmt/core.h>
#include <iostream>
#include <sstream>

using namespace dbpool::sqlite;

PreparedStmtImpl::PreparedStmtImpl(sqlite3 *db, sqlite3_stmt *stmt) noexcept
    : db_(db)
    , stmt_(stmt)
{
}

PreparedStmtImpl::~PreparedStmtImpl()
{
    if (stmt_) {
        if (sqlite3_bind_parameter_count(stmt_)) {
            sqlite3_clear_bindings(stmt_);
        }
        sqlite3_reset(stmt_);
    }
}

PreparedStmtImpl::ReturnCode PreparedStmtImpl::to_error_code(int code)
{
    switch (code) {
    case SQLITE_OK:
        return ReturnCode::ok;
    case SQLITE_ROW:
        return ReturnCode::row;
    case SQLITE_DONE:
        return ReturnCode::done;
    }
    assert(false);
    return ReturnCode::error;
}

PreparedStmtImpl::ReturnCode PreparedStmtImpl::execute()
{
    return to_error_code(sqlite3_step(stmt_));
}

void PreparedStmtImpl::bind_blob(int32_t const index, std::span<std::byte const> const &value)
{
    if (sqlite3_bind_blob(stmt_, index, &value[0], value.size_bytes(), SQLITE_TRANSIENT) != SQLITE_OK) {
        throw std::runtime_error(fmt::format("sqlite_stmt::bind_blob: {}", sqlite3_errmsg(db_)));
    }
}

void PreparedStmtImpl::bind_bool(int32_t const index, bool const value)
{
    if (sqlite3_bind_int(stmt_, index, value ? 1 : 0) != SQLITE_OK) {
        throw std::runtime_error(fmt::format("sqlite_stmt::bind_bool: {}", sqlite3_errmsg(db_)));
    }
}

void PreparedStmtImpl::bind_date(int32_t const index, std::string_view value)
{
    int ret{};
    if (value.empty()) {
        ret = sqlite3_bind_null(stmt_, index);
    }
    else {
        // Parse ISO 8601 date
        std::string const v(value);
        std::istringstream ss{v};
        date::sys_time<std::chrono::milliseconds> t;
        ss >> date::parse("%FT%TZ", t); // timestamp without TZ offset
        if (ss.fail()) {
            // Failed to parse the date; perhaps timestamp has a TZ offset, try again
            ss.clear();
            ss.str(v);
            ss >> date::parse("%FT%T%Ez", t); // timestamp with TZ offset
        }

        // Place number of milliseconds since epoch into database
        int64_t const millis = t.time_since_epoch().count();
        ret = sqlite3_bind_int(stmt_, index, millis);
    }

    if (ret != SQLITE_OK) {
        throw std::runtime_error(fmt::format("sqlite_stmt::bind_date: {}", sqlite3_errmsg(db_)));
    }
}

void PreparedStmtImpl::bind_double(int32_t const index, double const value)
{
    if (sqlite3_bind_double(stmt_, index, value) != SQLITE_OK) {
        throw std::runtime_error(fmt::format("sqlite_stmt::bind_double: {}", sqlite3_errmsg(db_)));
    }
}

void PreparedStmtImpl::bind_int32(int32_t const index, int32_t const value)
{
    if (sqlite3_bind_int(stmt_, index, value) != SQLITE_OK) {
        throw std::runtime_error(fmt::format("sqlite_stmt::bind_int32: {}", sqlite3_errmsg(db_)));
    }
}

void PreparedStmtImpl::bind_int64(int32_t const index, int64_t const value)
{
    if (sqlite3_bind_int64(stmt_, index, value) != SQLITE_OK) {
        throw std::runtime_error(fmt::format("sqlite_stmt::bind_int64: {}", sqlite3_errmsg(db_)));
    }
}

void PreparedStmtImpl::bind_null(int32_t const index)
{
    if (sqlite3_bind_null(stmt_, index) != SQLITE_OK) {
        throw std::runtime_error(fmt::format("sqlite_stmt::bind_null: {}", sqlite3_errmsg(db_)));
    }
}

void PreparedStmtImpl::bind_uuid(int32_t const index, std::span<std::byte const> const &value)
{
    if (sqlite3_bind_blob(stmt_, index, &value[0], value.size(), SQLITE_TRANSIENT) != SQLITE_OK) {
        throw std::runtime_error(fmt::format("sqlite_stmt::bind_blob: {}", sqlite3_errmsg(db_)));
    }
}

void PreparedStmtImpl::bind_text(int32_t const index, std::string_view value)
{
    auto const ret = value.empty() ? sqlite3_bind_null(stmt_, index)
                                   : sqlite3_bind_text(stmt_, index, value.data(), static_cast<int>(value.length()),
                                                       SQLITE_TRANSIENT);
    if (ret != SQLITE_OK) {
        throw std::runtime_error(fmt::format("sqlite::bind_text: {}", sqlite3_errmsg(db_)));
    }
}

void PreparedStmtImpl::bind_text16(int32_t const index, std::u16string_view value)
{
    auto const ret = value.empty() ? sqlite3_bind_null(stmt_, index)
                                   : sqlite3_bind_text16(stmt_, index, value.data(), static_cast<int>(value.length()),
                                                         SQLITE_TRANSIENT);
    if (ret != SQLITE_OK) {
        throw std::runtime_error(fmt::format("sqlite::bind_text16: {}", sqlite3_errmsg(db_)));
    }
}

std::vector<std::byte> PreparedStmtImpl::get_blob(int32_t const index)
{
    if (index < 0 || index >= sqlite3_column_count(stmt_)) {
        throw std::runtime_error(fmt::format("sqlite::column_blob: invalid index {}", index));
    }

    auto ptr = sqlite3_column_blob(stmt_, index);

    // nullptr may indicate an error; handle nullptr case
    if (!ptr) {
        if (sqlite3_errcode(db_) != SQLITE_OK) {
            throw std::runtime_error(fmt::format("sqlite::column_blob: out of memory exception {}", index));
        }
        return std::vector<std::byte>();
    }

    auto len = sqlite3_column_bytes(stmt_, index);
    std::vector<std::byte> buf(len);
    std::memcpy(buf.data(), ptr, len);
    return buf;
}

bool PreparedStmtImpl::get_bool(int32_t const index)
{
    if (index < 0 || index >= sqlite3_column_count(stmt_)) {
        throw std::runtime_error(fmt::format("sqlite::column_int: invalid index {}", index));
    }
    return sqlite3_column_int(stmt_, index) != 0;
}

std::string PreparedStmtImpl::get_date(int32_t const index)
{
    if (index < 0 || index >= sqlite3_column_count(stmt_)) {
        throw std::runtime_error(fmt::format("sqlite::column_int64: invalid index {}", index));
    }

    // Sqlite doesn't have a native date field; keep number of milliseconds since epoch in database
    auto millis = sqlite3_column_int64(stmt_, index);
    // Format timestamp
    std::chrono::milliseconds d{millis};
    date::sys_time<std::chrono::milliseconds> tp{d};
    return date::format("%FT%TZ", date::floor<std::chrono::milliseconds>(tp));
}

double PreparedStmtImpl::get_double(int32_t const index)
{
    if (index < 0 || index >= sqlite3_column_count(stmt_)) {
        throw std::runtime_error(fmt::format("sqlite::column_double: invalid index {}", index));
    }
    return sqlite3_column_double(stmt_, index);
}

int32_t PreparedStmtImpl::get_int32(int32_t const index)
{
    if (index < 0 || index >= sqlite3_column_count(stmt_)) {
        throw std::runtime_error(fmt::format("sqlite::column_int: invalid index {}", index));
    }
    return sqlite3_column_int(stmt_, index);
}

int64_t PreparedStmtImpl::get_int64(int32_t const index)
{
    if (index < 0 || index >= sqlite3_column_count(stmt_)) {
        throw std::runtime_error(fmt::format("sqlite::column_int32: invalid index {}", index));
    }
    return sqlite3_column_int64(stmt_, index);
}

std::string PreparedStmtImpl::get_text(int32_t const index)
{
    if (index < 0 || index >= sqlite3_column_count(stmt_)) {
        throw std::runtime_error(fmt::format("sqlite::column_text: invalid index {}", index));
    }

    auto ptr = sqlite3_column_text(stmt_, index);

    // nullptr may indicate an error; handle nullptr case
    if (!ptr) {
        if (sqlite3_errcode(db_) != SQLITE_OK) {
            throw std::runtime_error(fmt::format("sqlite::column_text: out of memory exception {}", index));
        }
        return std::string();
    }

    auto len = sqlite3_column_bytes(stmt_, index);
    std::string val(len, '\0');
    std::memcpy(val.data(), ptr, len);
    return val;
}

std::u16string PreparedStmtImpl::get_text16(int32_t const index)
{
    if (index < 0 || index >= sqlite3_column_count(stmt_)) {
        throw std::runtime_error(fmt::format("sqlite::column_text16: invalid index {}", index));
    }

    auto ptr = sqlite3_column_text16(stmt_, index);

    // nullptr may indicate an error; handle nullptr case
    if (!ptr) {
        if (sqlite3_errcode(db_) != SQLITE_OK) {
            throw std::runtime_error(fmt::format("sqlite::column_text16: {}", sqlite3_errmsg(db_)));
        }
        return std::u16string();
    }

    auto len = sqlite3_column_bytes16(stmt_, index);
    std::u16string val(len, '\0');
    std::memcpy(val.data(), ptr, len);
    return val;
}

std::array<uint8_t, 16> PreparedStmtImpl::get_uuid(int32_t const index)
{
    if (index < 0 || index >= sqlite3_column_count(stmt_)) {
        throw std::runtime_error(fmt::format("sqlite::column_blob: invalid index {}", index));
    }

    std::array<uint8_t, 16> blob;
    if (auto b = sqlite3_column_blob(stmt_, index)) {
        memcpy((void *)&blob[0], b, blob.size());
    }
    return blob;
}
