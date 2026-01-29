#ifndef DBPOOL_SQLITE_CONNECTION_POOL_HPP
#define DBPOOL_SQLITE_CONNECTION_POOL_HPP

#include "dbpool/ConnectionPool.hpp"
#include "dbpool/DatabaseFile.hpp"
#include <memory>

#include <atomic>
#include <chrono>
#include <sqlite3.h>
#include <thread>

namespace dbpool::sqlite {

//!
//! \class ConnectionPool
//! \brief Implements the \c ConnectionPool interface for an SQLite database connection pool.
//!
//! This class implements the factory functions needed by the \c ConnectionPool class to construct SQLite database
//! connections that will be managed by the \c ConnectionPool class.
//!
//! This object keeps a file handle to the SQLite database file that it manages the connection pool for. This connection
//! isn't counted in the total number of connections for the pool and is used to performs housekeeping on the underlying
//! database backend such as: vacuuming (defragmentation) upon database close and periodic database optimization. Keep
//! this in mind as there will be one more connection than specified for this connection pool.
//!
class ConnectionPool
    : public dbpool::ConnectionPool
    , public dbpool::DatabaseFile
    , public std::enable_shared_from_this<ConnectionPool> {
public:
    using optimization_period_t = std::chrono::minutes;

public:
    ConnectionPool(ConnectionPool const &) = delete;
    ConnectionPool(ConnectionPool &&) noexcept = delete;
    explicit ConnectionPool(std::string_view filename, size_t pool_size = 15,
                            optimization_period_t optimization_period = optimization_period_t(10),
                            size_t analysis_limit = 400);
    ~ConnectionPool() override;

    ConnectionPool &operator=(ConnectionPool const &) = delete;
    ConnectionPool &operator=(ConnectionPool &&) noexcept = delete;

    void commit() override;
    std::string get_filename() const override;
    bool is_open() const override;

    int64_t get_schema() override;
    void set_schema(int64_t schema) override;

protected:
    std::unique_ptr<dbpool::ConnectionImpl> new_conn() override;
    std::shared_ptr<dbpool::ConnectionPool> shared_base_ptr() override;

private:
    void start_optimization_thread(optimization_period_t period, size_t threshold);
    void stop_optimization_thread();
    void optimization_thread(sqlite3_stmt *stmt, optimization_period_t period, size_t threshold);

    void initialize_sqlite();
    void shutdown_sqlite();

    std::mutex sqlite_init_mutex_;
    static size_t sqlite_use_count_;

    std::unique_ptr<std::thread> m_optimization_thread;
    std::mutex m_optimization_thread_mutex;

    std::atomic<bool> m_run_optimization_thread{false};
    std::mutex m_optimization_mutex;
    std::condition_variable m_optimization_condition;

    sqlite3 *m_db{nullptr};
};

} // namespace dbpool::sqlite

#endif
