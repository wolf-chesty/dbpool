// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include "dbpool/sqlite/ConnectionPoolImpl.hpp"

#include "dbpool/sqlite/ConnectionImpl.hpp"
#include <cassert>
#include <fmt/core.h>
#include <sqlite3.h>
#include <string>

using namespace dbpool::sqlite;

// static
// void sqlite3_hook(void * /*object*/, int op, const char * /*database*/, const char * /*table*/,
// sqlite_int64 /*key*/)
//{
//	switch (op)
//	{
//	case SQLITE_INSERT:
//		break;
//
//	case SQLITE_DELETE:
//		break;
//
//	case SQLITE_UPDATE:
//		break;
//
//	default:
//		break;
//	}
// }

/*
static
int exec_callback(void *data, int c_num, char **c_vals, char **c_names)
{
        TRACE("%s:\n", static_cast<const char *>(data));
        for (decltype(c_num) i = 0; i < c_num; ++i)
        {
                TRACE("\t%s = %s\n", c_names + i, c_vals[i] ? c_vals[i] : "nullptr");
        }
        TRACE("\n");
        return 0;
}
*/

size_t ConnectionPoolImpl::sqlite_use_count_ = 0;

ConnectionPoolImpl::ConnectionPoolImpl(std::string_view filename, size_t pool_size,
                                       optimization_period_t optimization_period, size_t analysis_limit)
    : dbpool::ConnectionPoolImpl(pool_size)
{
    initializeSqlite();

    // Open handle to database for connection pool related stuff
    if (SQLITE_OK != sqlite3_open(filename.data(), &db_)) {
        sqlite3_close(db_);
        shutdownSqlite();
        throw std::runtime_error(fmt::format("Unable to open file '{}'", filename));
    }
}

ConnectionPoolImpl::~ConnectionPoolImpl()
{
    assert(db_);

    ConnectionPoolImpl::commit();
    sqlite3_close(db_);

    shutdownSqlite();
}

void ConnectionPoolImpl::initializeSqlite()
{
    std::lock_guard const lock(sqlite_init_mutex_);
    if (sqlite_use_count_++ == 0) {
        if (auto const ret = sqlite3_config(SQLITE_CONFIG_MULTITHREAD, nullptr); SQLITE_OK != ret) {
            throw std::runtime_error(fmt::format("Unable to configure sqlite3; got return code {}", ret));
        }
        if (auto const ret = sqlite3_initialize(); SQLITE_OK != ret) {
            throw std::runtime_error(fmt::format("Unable to initialize sqlite3; got return code {}", ret));
        }
    }
}

void ConnectionPoolImpl::shutdownSqlite()
{
    std::lock_guard const lock(sqlite_init_mutex_);
    if (--sqlite_use_count_ == 0) {
#ifdef NDEBUG
        sqlite3_shutdown();
#else
        assert(SQLITE_OK == sqlite3_shutdown());
#endif
    }
}

int64_t ConnectionPoolImpl::getSchema()
{
    assert(db_);

    std::lock_guard const lock(db_mutex_);

    // Compile prepared statement
    std::string_view sql{"PRAGMA user_version"};
    sqlite3_stmt *stmt{};
    if (sqlite3_prepare_v2(db_, sql.data(), sql.length() + 1, &stmt, nullptr)) {
        throw std::runtime_error(fmt::format("sqlite_conn_pool::getSchema: {}", sqlite3_errmsg(db_)));
    }

    // Execute the prepared statement
#ifdef NDEBUG
    sqlite3_step(stmt);
#else
    assert(SQLITE_ROW == sqlite3_step(stmt));
#endif

    auto const schema = sqlite3_column_int64(stmt, 0);
    sqlite3_finalize(stmt);

    return schema;
}

void ConnectionPoolImpl::setSchema(int64_t schema)
{
    assert(db_);
    auto const sqlStmt = fmt::format("PRAGMA user_version = {}", schema);
    std::lock_guard const lock(db_mutex_);
    if (sqlite3_exec(db_, sqlStmt.data(), nullptr, nullptr, nullptr)) {
        throw std::runtime_error(fmt::format("sqlite_conn_pool::setSchema: {}", sqlite3_errmsg(db_)));
    }
}

std::unique_ptr<dbpool::ConnectionImpl> ConnectionPoolImpl::createConnection()
{
    sqlite3 *db{};
    auto filename = getFilename();
    if (sqlite3_open(filename.c_str(), &db)) {
        throw std::runtime_error(fmt::format("sqlite_conn_pool::createConnection: Unable to open file '{}'", filename));
    }
    return std::make_unique<ConnectionImpl>(db);
}

void ConnectionPoolImpl::commit()
{
    assert(db_);
    std::lock_guard const lock(db_mutex_);
    if (sqlite3_exec(db_, "VACUUM", nullptr, nullptr, nullptr)) {
        throw std::runtime_error(fmt::format("sqlite_conn_pool::commit: {}", sqlite3_errmsg(db_)));
    }
}

std::string ConnectionPoolImpl::getFilename() const
{
    assert(db_);
    auto f = sqlite3_db_filename(db_, nullptr);
    return std::string{!f || strlen(f) == 0 ? ":memory:" : f};
}

bool ConnectionPoolImpl::isOpen() const
{
    return db_ != nullptr;
}
