#include "sqlite/sqlite_conn_impl.h"

#include <cassert>
#include <fmt/core.h>

#include "sqlite/sqlite_stmt.h"

using namespace dbpool;

//!
//! \brief Creates an \c sqlite_conn_impl wrapper around an \c sqlite3 connection pointer.
//!
//! \param db SQLite3 API database connection.
//!
sqlite_conn_impl::sqlite_conn_impl(sqlite3 *db)
    : mDb(db)
{
}

//!
//! \brief Destroys the \c sqlite_conn_impl connection class and resets any cached compiled
//! statements.
//!
sqlite_conn_impl::~sqlite_conn_impl()
{
    // Make sure to clean up memory for cached prepared statements
    for (auto &stmt : mStmtCache) {
        sqlite3_finalize(stmt.second);
    }

    // Close database handle
    assert(mDb);
    sqlite3_close(mDb);
}

//!
//! \brief Executes statement \c sql on the database connection.
//!
//! \param sql SQL statement to execute.
//! \return Return code for statement executed.
//!
db_stmt::return_code sqlite_conn_impl::exec(std::string_view sql)
{
    assert(mDb);
    return sqlite_stmt::to_error_code(sqlite3_exec(mDb, sql.data(), nullptr, nullptr, nullptr));
}

//!
//! \brief Returns a prepared statement for this database connection.
//!
//! \param conn Connection guard that initiated the construction of the prepared statement.
//! \param sql SQL statement of the prepared statement.
//! \return Pointer to a prepared statement.
//!
std::unique_ptr<db_stmt> sqlite_conn_impl::get_stmt(std::shared_ptr<db_conn> conn, std::string const &sql)
{
    assert(mDb);

    auto it = mStmtCache.find(sql);
    if (it != mStmtCache.end()) {
        return std::make_unique<sqlite_stmt>(conn, mDb, it->second);
    }

    // Create new stored procedure for connection
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(mDb, sql.data(), sql.length() + 1, &stmt, nullptr))
        throw std::runtime_error(fmt::format("sqlite_conn_impl::get_stmt: {}", sqlite3_errmsg(mDb)));

    mStmtCache.emplace(std::make_pair(sql, stmt));
    return std::make_unique<sqlite_stmt>(conn, mDb, stmt);
}
