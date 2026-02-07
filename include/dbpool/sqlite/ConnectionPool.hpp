// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#ifndef DBPOOL_SQLITE_CONNECTIONPOOL_HPP
#define DBPOOL_SQLITE_CONNECTIONPOOL_HPP

#include "dbpool/ConnectionPool.hpp"
#include "dbpool/DatabaseFile.hpp"

#include "dbpool/sqlite/ConnectionPoolImpl.hpp"

namespace dbpool::sqlite {

///
/// @class ConnectionPool
///
/// @brief Class provides additional interfaces for managing SQLite3 type database. Along with the normal
///        \c ConnectionPool specific interfaces SQLite3 databases must implement the \c DatabaseFile interface.
///
class ConnectionPool
    : public dbpool::ConnectionPool<ConnectionPoolImpl>
    , public dbpool::DatabaseFile {
public:
    using optimization_period_t = ConnectionPoolImpl::optimization_period_t;

public:
    ConnectionPool() = delete;
    ConnectionPool(ConnectionPool const &) = delete;
    ConnectionPool(ConnectionPool &&) noexcept = default;

    /// @brief Creates a \c ConnectionPool object for the database named \c filename.
    ///
    /// @param filename Database filename.
    /// @param pool_size Number of connections in the database connection pool.
    /// @param optimization_period Number of minutes to wait between database optimizations.
    /// @param analysis_limit Number of records to process during database optimizations.
    explicit ConnectionPool(std::string_view filename, size_t pool_size = 15,
                            optimization_period_t optimization_period = optimization_period_t(10),
                            size_t analysis_limit = 400);

    ~ConnectionPool() override = default;

    ConnectionPool &operator=(ConnectionPool const &) = delete;
    ConnectionPool &operator=(ConnectionPool &&) = delete;

    /// @brief Flushes database changes from memory to disk.
    void commit() override;

    /// @brief Returns the database filename.
    ///
    /// @return Database filename.
    std::string get_filename() const override;

    /// @brief Returns \c true if the database is open.
    ///
    /// @return \c true if the database is open.
    bool is_open() const override;
};

} // namespace dbpool::sqlite

#endif
