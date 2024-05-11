#pragma once

#include <atomic>
#include <sqlite3.h>
#include <thread>

#include "db_conn_pool.h"
#include "db_file.h"

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
    sqlite_conn_pool(sqlite_conn_pool const &) = delete;
    sqlite_conn_pool(sqlite_conn_pool &&) = delete;

    ~sqlite_conn_pool() override;

    sqlite_conn_pool &operator=(sqlite_conn_pool const &) = delete;
    sqlite_conn_pool &operator=(sqlite_conn_pool &&) = delete;

    static std::shared_ptr<db_conn_pool> create(std::string_view filename, size_t const poolSize = defaultPoolSize,
                                                size_t const optimizationTimeout = defaultOptimizationTimeout);

    void commit() override;
    std::string get_filename() const override;
    bool is_open() const override;

    int64_t get_schema() override;
    void set_schema(int64_t const schema) override;

protected:
    std::unique_ptr<db_conn_impl> new_conn() override;
    std::shared_ptr<db_conn_pool> shared_base_ptr() override;

private:
    sqlite_conn_pool(std::string_view filename, size_t const poolSize, size_t const optimizationTimeout);

    void start_optimization_thread(size_t const timeout, size_t const threshold);
    void stop_optimization_thread();
    void optimization_thread(sqlite3_stmt *stmt, size_t const timeout, size_t const threshold);

    std::unique_ptr<std::thread> mOptimizationThread;
    std::mutex mOptimizationThreadMutex;

    bool mRunOptimizationThread{false};
    std::mutex mOptimizationMutex;
    std::condition_variable mOptimizationCondition;

    static size_t const defaultPoolSize;
    static size_t const defaultOptimizationTimeout;

    sqlite3 *mDb{};
};
