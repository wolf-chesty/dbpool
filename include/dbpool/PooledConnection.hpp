// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#ifndef DBPOOL_CONNECTION_SCOPE_HPP
#define DBPOOL_CONNECTION_SCOPE_HPP

#include <memory>

namespace dbpool {

class ConnectionImpl;
class ConnectionPoolImpl;

///
/// @class PooledConnection
///
/// @brief Object is responsible for returning a checked out database connection to the connection pool that created it.
///
class PooledConnection {
public:
    PooledConnection() = delete;
    PooledConnection(PooledConnection const &) = delete;
    PooledConnection(PooledConnection &&) noexcept = default;

    /// @brief ctor.
    ///
    /// @param conn_pool Connection pool that created \c conn.
    /// @param conn Database connection to be returned to \c conn_pool up destruction.
    PooledConnection(std::shared_ptr<ConnectionPoolImpl> conn_pool, std::unique_ptr<ConnectionImpl> conn);

    /// @brief Returns database connection to connection pool.
    ~PooledConnection();

    PooledConnection &operator=(PooledConnection const &) = delete;
    PooledConnection &operator=(PooledConnection &&) noexcept = default;

    /// @brief Returns pointer to database connection.
    ///
    /// @return Pointer to database connection.
    ConnectionImpl *get_conn();

private:
    std::shared_ptr<ConnectionPoolImpl> conn_pool_; ///< Connection pool to return connection to.
    std::unique_ptr<ConnectionImpl> conn_;          ///< Database connection to return.
};

} // namespace dbpool

#endif
