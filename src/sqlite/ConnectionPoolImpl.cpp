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
    initialize_sqlite();

    // Open handle to database for connection pool related stuff
    if (SQLITE_OK != sqlite3_open(filename.data(), &db_)) {
        throw std::runtime_error(fmt::format("Unable to open file '{}'", filename));
    }

    start_optimization_thread(optimization_period, analysis_limit);
}

ConnectionPoolImpl::~ConnectionPoolImpl()
{
    assert(db_);

    stop_optimization_thread();
    ConnectionPoolImpl::commit();
    sqlite3_close(db_);

    shutdown_sqlite();
}

void ConnectionPoolImpl::initialize_sqlite()
{
    std::unique_lock const lock(sqlite_init_mutex_);
    if (sqlite_use_count_++ == 0) {
        if (auto const ret = sqlite3_config(SQLITE_CONFIG_MULTITHREAD, nullptr); SQLITE_OK != ret) {
            throw std::runtime_error(fmt::format("Uanble to configure sqlite3; got return code {}", ret));
        }
        if (auto const ret = sqlite3_initialize(); SQLITE_OK != ret) {
            throw std::runtime_error(fmt::format("Unable to initialize sqlite3; got return code {}", ret));
        }
    }
}

void ConnectionPoolImpl::shutdown_sqlite()
{
    std::unique_lock const lock(sqlite_init_mutex_);
    if (--sqlite_use_count_ == 0) {
#ifdef NDEBUG
        sqlite3_shutdown();
#else
        assert(SQLITE_OK == sqlite3_shutdown());
#endif
    }
}

int64_t ConnectionPoolImpl::get_schema()
{
    assert(db_);

    std::unique_lock const lock(db_mutex_);

    // Compile prepared statement
    std::string_view sql{"PRAGMA user_version"};
    sqlite3_stmt *stmt{};
    if (sqlite3_prepare_v2(db_, sql.data(), sql.length() + 1, &stmt, nullptr)) {
        throw std::runtime_error(fmt::format("sqlite_conn_pool::get_schema: {}", sqlite3_errmsg(db_)));
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

void ConnectionPoolImpl::set_schema(int64_t schema)
{
    assert(db_);
    auto const sqlStmt = fmt::format("PRAGMA user_version = {}", schema);
    std::unique_lock const lock(db_mutex_);
    if (sqlite3_exec(db_, sqlStmt.data(), nullptr, nullptr, nullptr)) {
        throw std::runtime_error(fmt::format("sqlite_conn_pool::set_schema: {}", sqlite3_errmsg(db_)));
    }
}

std::unique_ptr<dbpool::ConnectionImpl> ConnectionPoolImpl::new_conn()
{
    sqlite3 *db{};
    auto filename = get_filename();
    if (sqlite3_open(filename.c_str(), &db)) {
        throw std::runtime_error(fmt::format("sqlite_conn_pool::new_conn: Unable to open file '{}'", filename));
    }
    return std::make_unique<ConnectionImpl>(db);
}

void ConnectionPoolImpl::optimization_thread(optimization_period_t period, [[maybe_unused]] size_t threshold)
{
    assert(period.count() > 0);

    try {
        sqlite3_stmt *stmt{};
        auto const sql = fmt::format("PRAGMA analysis_limit = {}; PRAGMA optimize;", threshold);
        if (sqlite3_prepare_v2(db_, sql.data(), sql.length() + 1, &stmt, nullptr)) {
            throw std::runtime_error(fmt::format("sqlite_conn_pool::optimization_thread: {}", sqlite3_errmsg(db_)));
        }

        do {
            std::unique_lock lock(db_mutex_);
            optimization_cv_.wait_for(lock, period, [this]() -> bool { return !run_optimization_thread_; });

            sqlite3_step(stmt);
            sqlite3_reset(stmt);
        } while (run_optimization_thread_);

        sqlite3_finalize(stmt);
    }
    catch (std::exception const &e) {
    }
}

void ConnectionPoolImpl::start_optimization_thread(optimization_period_t period, size_t threshold)
{
    if (period.count() > 0 && !run_optimization_thread_.exchange(true)) {
        optimization_thread_ =
            std::thread(&ConnectionPoolImpl::optimization_thread, this, std::move(period), threshold);

        std::string_view thread_name("sqlite_opt");
        assert(thread_name.length() <= 16);
        pthread_setname_np(optimization_thread_.native_handle(), thread_name.data());
    }
}

void ConnectionPoolImpl::stop_optimization_thread()
{
    if (run_optimization_thread_.exchange(false)) {
        optimization_cv_.notify_one();
    }
    optimization_thread_.join();
}

void ConnectionPoolImpl::commit()
{
    assert(db_);
    std::unique_lock const lock(db_mutex_);
    if (sqlite3_exec(db_, "VACUUM", nullptr, nullptr, nullptr)) {
        throw std::runtime_error(fmt::format("sqlite_conn_pool::commit: {}", sqlite3_errmsg(db_)));
    }
}

std::string ConnectionPoolImpl::get_filename() const
{
    auto f = sqlite3_db_filename(db_, nullptr);
    return std::string{!f || strlen(f) == 0 ? ":memory:" : f};
}

bool ConnectionPoolImpl::is_open() const
{
    return db_ != nullptr;
}
