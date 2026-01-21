#pragma once

#include <atomic>
#include <chrono>
#include <sqlite3.h>
#include <thread>

#include "dbpool/db_conn_pool.h"
#include "dbpool/db_file.h"

namespace dbpool {

//!
//! \class sqlite_conn_pool
//! \brief Implements the \c db_conn_pool interface for an SQLite database connection pool.
//!
//! This class implements the factory functions needed by the \c db_conn_pool class to construct SQLite database
//! connections that will be managed by the \c db_conn_pool class.
//!
//! This object keeps a file handle to the SQLite database file that it manages the connection pool for. This connection
//! isn't counted in the total number of connections for the pool and is used to performs housekeeping on the underlying
//! database backend such as: vacuuming (defragmentation) upon database close and periodic database optimization. Keep
//! this in mind as there will be one more connection than specified for this connection pool.
//!
class sqlite_conn_pool
    : public db_conn_pool
    , public db_file
    , public std::enable_shared_from_this<sqlite_conn_pool> {
public:
    using optimization_period_t = std::chrono::minutes;

public:
    sqlite_conn_pool(sqlite_conn_pool const &) = delete;
    sqlite_conn_pool(sqlite_conn_pool &&) = delete;
    sqlite_conn_pool(std::string_view filename, size_t pool_size, optimization_period_t optimization_period);

    ~sqlite_conn_pool() override;

    sqlite_conn_pool &operator=(sqlite_conn_pool const &) = delete;
    sqlite_conn_pool &operator=(sqlite_conn_pool &&) = delete;

    static std::shared_ptr<db_conn_pool> create(std::string_view filename, size_t pool_size = 15,
                                                optimization_period_t optimization_period = optimization_period_t(10));

    void commit() override;
    std::string get_filename() const override;
    bool is_open() const override;

    int64_t get_schema() override;
    void set_schema(int64_t schema) override;

protected:
    std::unique_ptr<db_conn_impl> new_conn() override;
    std::shared_ptr<db_conn_pool> shared_base_ptr() override;

private:
    void start_optimization_thread(optimization_period_t period, size_t threshold);
    void stop_optimization_thread();
    void optimization_thread(sqlite3_stmt *stmt, optimization_period_t period, size_t threshold);

    std::unique_ptr<std::thread> m_optimization_thread;
    std::mutex m_optimization_thread_mutex;

    std::atomic<bool> m_run_optimization_thread{false};
    std::mutex m_optimization_mutex;
    std::condition_variable m_optimization_condition;

    sqlite3 *m_db{nullptr};
};

} // namespace dbpool
