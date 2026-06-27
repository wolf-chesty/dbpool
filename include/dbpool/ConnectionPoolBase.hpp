// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#ifndef DBPOOL_CONNECTION_POOL_BASE_HPP
#define DBPOOL_CONNECTION_POOL_BASE_HPP

#include "dbpool/Connection.hpp"

namespace dbpool {

/// @class ConnectionPoolBase
///
/// @brief Database connection pool base.
///
/// The resposibility of this class is to provide a base class to a connection pool.
class ConnectionPoolBase {
public:
    virtual ~ConnectionPoolBase() = default;

    /// @brief Returns a new \c Connection object that can be used to execute statements against the database.
    virtual Connection getConnection() = 0;
};

} // namespace dbpool

#endif
