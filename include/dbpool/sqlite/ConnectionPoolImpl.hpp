// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#ifndef DBPOOL_SQLITE_CONNECTIONPOOLIMPL_HPP
#define DBPOOL_SQLITE_CONNECTIONPOOLIMPL_HPP

#include "dbpool/DatabaseFile.hpp"
#include "dbpool/impl/ConnectionPoolImpl.hpp"
#include <memory>

#include <atomic>
#include <chrono>
#include <sqlite3.h>
#include <thread>

namespace dbpool::sqlite {
/// @class ConnectionPoolImpl
///
/// @brief Implements the \c ConnectionPool interface for an SQLite database connection pool.
///
/// This class implements the factory functions needed by the \c ConnectionPool class to construct SQLite database
/// connections that will be managed by the \c ConnectionPool class.
///
/// This object keeps a file handle to the SQLite database file that it manages the connection pool for. This connection
/// isn't counted in the total number of connections for the pool and is used to performs housekeeping on the underlying
/// database backend such as: vacuuming (defragmentation) upon database close and periodic database optimization. Keep
/// this in mind as there will be one more connection than specified for this connection pool.
class ConnectionPoolImpl
    : public dbpool::ConnectionPoolImpl
    , public dbpool::DatabaseFile {
public:
    using optimization_period_t = std::chrono::minutes;

public:
    ConnectionPoolImpl(ConnectionPoolImpl const &) = delete;
    ConnectionPoolImpl(ConnectionPoolImpl &&) noexcept = delete;

    /// @brief Creates a \c ConnectionPool object for the database named \c filename.
    ///
    /// @param filename Database filename.
    /// @param pool_size Number of connections in the database connection pool.
    /// @param optimization_period Number of minutes to wait between database optimizations.
    /// @param analysis_limit Number of records to process during database optimizations.
    explicit ConnectionPoolImpl(std::string_view filename, size_t pool_size = 15,
                                optimization_period_t optimization_period = optimization_period_t(10),
                                size_t analysis_limit = 400);

    /// @brief dtor for connection pool.
    ~ConnectionPoolImpl() override;

    ConnectionPoolImpl &operator=(ConnectionPoolImpl const &) = delete;
    ConnectionPoolImpl &operator=(ConnectionPoolImpl &&) noexcept = delete;

    /// @brief Defragments the database file, removing empty spaces caused by record deletions.
    void commit() override;

    /// @brief Returns the database filename.
    ///
    /// @return Database filename.
    std::string getFilename() const override;

    /// @brief Returns \c true if the database is open.
    ///
    /// @return \c true if the database is open.
    bool isOpen() const override;

    /// @brief Returns the schema number of the database.
    ///
    /// @return Database schema number.
    int64_t getSchema() override;

    /// @brief Sets the schema number for the database.
    ///
    /// @param schema Database schema number.
    void setSchema(int64_t schema) override;

protected:
    /// @brief Creates and returns a pointer of a new \c ConnectionImpl object for the managed database.
    ///
    /// @return Pointer to a new \c ConnectionImpl object.
    std::unique_ptr<dbpool::ConnectionImpl> createConnection() override;

private:
    /// @brief Starts the optimization thread.
    ///
    /// @param period Number of minutes to wait between calls to the optimization thread.
    /// @param threshold Number of records to optimize at once.
    void startOptimizationThread(optimization_period_t period, size_t threshold);

    /// @brief Stops the optimization thread.
    ///
    /// If the optimization thread was started for this connection pool then this function will stop that thread.
    /// Otherwise, this function is effectively a no-op and does nothing.
    void stopOptimizationThread();

    /// @brief Function that implements the thread that periodically optimizes the database.
    ///
    /// @param period Number of minutes to wait between optimization calls.
    /// @param threshold Number of records to examine when performing the optimization.
    ///
    /// @note Time taken to optimize the database does not count towards the timeout length.
    ///
    /// This function will wake up periodically and perform an optimization on the SQLite database file.
    void optimizationThread(optimization_period_t period, size_t threshold);

    /// @brief Initializes the sqlite3 library for use by the application.
    void initializeSqlite();

    /// @brief Uninitializes the sqlite3 library for use by the application.
    void shutdownSqlite();

    std::mutex sqlite_init_mutex_;
    static size_t sqlite_use_count_;

    std::thread optimization_thread_;
    std::condition_variable optimization_cv_;
    std::atomic<bool> run_optimization_thread_{false};

    sqlite3 *db_{nullptr};
    std::mutex db_mutex_;
};
} // namespace dbpool::sqlite

#endif
