#include "dbpool/sqlite/sqlite_stmt.h"

#include <cassert>
#include <chrono>
#include <cstring>
#include <date/date.h>
#include <fmt/core.h>
#include <iostream>
#include <sstream>

using namespace dbpool;

//!
//! \brief Constructs a prepared statement for connection \c db using SQL statement \c stmt.
//!
//! \param db Database connection that this prepared statement is bound to.
//! \param stmt SQL statement to create prepared statement from.
//!
sqlite_stmt::sqlite_stmt(std::shared_ptr<db_conn> conn, sqlite3 *db, sqlite3_stmt *stmt) noexcept
    : db_stmt(conn)
    , m_db(db)
    , m_stmt(stmt)
{
}

//!
//! \brief Resets the underlying prepared statement for reuse.
//!
//! Prepared statements are cached for reuse so rather than actually freeing the prepared statement
//! memory this function will unbind and reset the prepared statement to its initial state for reuse
//! by another process. Prepared statement deletion will actually be performed by the database
//! connection that created it.
//!
sqlite_stmt::~sqlite_stmt()
{
    assert(m_stmt);

    if (sqlite3_bind_parameter_count(m_stmt)) {
        sqlite3_clear_bindings(m_stmt);
    }
    sqlite3_reset(m_stmt);
}

sqlite_stmt::return_code sqlite_stmt::to_error_code(int code)
{
    switch (code) {
    case SQLITE_OK:
        return return_code::ok;
    case SQLITE_ROW:
        return return_code::row;
    case SQLITE_DONE:
        return return_code::done;
    }
    return return_code::error;
}

sqlite_stmt::return_code sqlite_stmt::execute()
{
    return to_error_code(sqlite3_step(m_stmt));
}

void sqlite_stmt::bind_blob(int32_t const index, std::span<std::byte const> const &value)
{
    if (sqlite3_bind_blob(m_stmt, index, &value[0], value.size_bytes(), SQLITE_TRANSIENT)) {
        throw std::runtime_error(fmt::format("sqlite_stmt::bind_blob: {}", sqlite3_errmsg(m_db)));
    }
}

void sqlite_stmt::bind_bool(int32_t const index, bool const value)
{
    if (sqlite3_bind_int(m_stmt, index, value ? 1 : 0)) {
        throw std::runtime_error(fmt::format("sqlite_stmt::bind_bool: {}", sqlite3_errmsg(m_db)));
    }
}

void sqlite_stmt::bind_date(int32_t const index, std::string_view value)
{
    int ret{};
    if (value.empty()) {
        ret = sqlite3_bind_null(m_stmt, index);
    }
    else {
        // TODO:
        //  We need to evaluate this code; according to sources on the internet parsing dates using
        //  the standard template library can be slower than parsing the JSON data. Perhaps
        //  switching to a database with native ISO 8601 date support would allow us to offload date
        //  parsing to another process? In the mean time, since parsing	happens on a separate thread
        //  we can safely assume that the web interface isn't hung up waiting for date parsing to
        //  complete.

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
        ret = sqlite3_bind_int(m_stmt, index, millis);
    }

    if (ret) {
        throw std::runtime_error(fmt::format("sqlite_stmt::bind_date: {}", sqlite3_errmsg(m_db)));
    }
}

void sqlite_stmt::bind_double(int32_t const index, double const value)
{
    if (sqlite3_bind_double(m_stmt, index, value)) {
        throw std::runtime_error(fmt::format("sqlite_stmt::bind_double: {}", sqlite3_errmsg(m_db)));
    }
}

void sqlite_stmt::bind_int32(int32_t const index, int32_t const value)
{
    if (sqlite3_bind_int(m_stmt, index, value)) {
        throw std::runtime_error(fmt::format("sqlite_stmt::bind_int32: {}", sqlite3_errmsg(m_db)));
    }
}

void sqlite_stmt::bind_int64(int32_t const index, int64_t const value)
{
    if (sqlite3_bind_int64(m_stmt, index, value)) {
        throw std::runtime_error(fmt::format("sqlite_stmt::bind_int64: {}", sqlite3_errmsg(m_db)));
    }
}

void sqlite_stmt::bind_null(int32_t const index)
{
    if (sqlite3_bind_null(m_stmt, index)) {
        throw std::runtime_error(fmt::format("sqlite_stmt::bind_null: {}", sqlite3_errmsg(m_db)));
    }
}

void sqlite_stmt::bind_uuid(int32_t const index, std::span<std::byte const> const &value)
{
    if (sqlite3_bind_blob(m_stmt, index, &value[0], value.size(), SQLITE_TRANSIENT)) {
        throw std::runtime_error(fmt::format("sqlite_stmt::bind_blob: {}", sqlite3_errmsg(m_db)));
    }
}

void sqlite_stmt::bind_text(int32_t const index, std::string_view value)
{
    auto const ret = value.empty() ? sqlite3_bind_null(m_stmt, index)
                                   : sqlite3_bind_text(m_stmt, index, value.data(), static_cast<int>(value.length()),
                                                       SQLITE_TRANSIENT);
    if (ret) {
        throw std::runtime_error(fmt::format("sqlite::bind_text: {}", sqlite3_errmsg(m_db)));
    }
}

bool sqlite_stmt::get_bool(int32_t const index)
{
    return sqlite3_column_int(m_stmt, index) != 0;
}

std::string sqlite_stmt::get_date(int32_t const index)
{
    // Sqlite doesn't have a native date field; keep number of milliseconds since epoch in database
    auto millis = sqlite3_column_int64(m_stmt, index);
    // Format timestamp
    std::chrono::milliseconds d{millis};
    date::sys_time<std::chrono::milliseconds> tp{d};
    return date::format("%FT%TZ", date::floor<std::chrono::milliseconds>(tp));
}

double sqlite_stmt::get_double(int32_t const index)
{
    return sqlite3_column_double(m_stmt, index);
}

int32_t sqlite_stmt::get_int32(int32_t const index)
{
    return sqlite3_column_int(m_stmt, index);
}

int64_t sqlite_stmt::get_int64(int32_t const index)
{
    return sqlite3_column_int64(m_stmt, index);
}

std::string sqlite_stmt::get_text(int32_t const index)
{
    return reinterpret_cast<char const *>(sqlite3_column_text(m_stmt, index));
}

std::array<uint8_t, 16> sqlite_stmt::get_uuid(int32_t const index)
{
    std::array<uint8_t, 16> blob;
    if (auto b = sqlite3_column_blob(m_stmt, index)) {
        memcpy((void *)&blob[0], b, blob.size());
    }
    return blob;
}
