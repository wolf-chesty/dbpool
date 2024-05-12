#pragma once

#include <map>
#include <memory>
#include <sqlite3.h>
#include <string>

#include "db_conn_impl.h"

namespace dbpool {

//!
//! \class sqlite_conn_impl
//! \brief Class that implements a connection to an SQLite3 database.
//!
class sqlite_conn_impl : public db_conn_impl {
public:
    using stmt_cache_type = std::map<std::string, sqlite3_stmt *>;

public:
    sqlite_conn_impl(sqlite_conn_impl const &) = delete;
    sqlite_conn_impl(sqlite_conn_impl &&) = delete;
    explicit sqlite_conn_impl(sqlite3 *db);

    ~sqlite_conn_impl() override;

    sqlite_conn_impl &operator=(sqlite_conn_impl const &) = delete;
    sqlite_conn_impl &operator=(sqlite_conn_impl &&conn) = delete;

    db_stmt::return_code exec(std::string_view sql) override;
    std::unique_ptr<db_stmt> get_stmt(std::shared_ptr<db_conn> conn, std::string const &sql) override;

private:
    sqlite3 *mDb{};
    stmt_cache_type mStmtCache;
};

} // namespace dbpool
