// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#ifndef DBPOOL_CONNECTION_HPP
#define DBPOOL_CONNECTION_HPP

#include "dbpool/PreparedStmt.hpp"
#include <memory>

namespace dbpool {
class PooledConnection;

/// @class Connection
///
/// @brief This class wraps the concrete implementations of \c ConnectionImpl and will return the implementation back to
///        the managing \c ConnectionPool.
///
/// This object manages the concrete database connection implementations and is responsible for returning unused
/// database connections to the connection pool object that created them. This object delegates all database
/// functionality to the underlying concrete \c ConnectionImpl class which implements any API specific database code.
class Connection {
public:
    Connection() = default;
    Connection(Connection const &) = delete;
    Connection(Connection &&) noexcept = default;

    /// @brief ctor for the object.
    ///
    /// @param pooled_conn Object holding the connection implementation and connection pool.
    explicit Connection(std::shared_ptr<PooledConnection> pooled_conn);

    virtual ~Connection() = default;

    Connection &operator=(Connection const &) = delete;
    Connection &operator=(Connection &&) noexcept = default;

    /// @brief Executes an SQL statements on the managed database connection.
    ///
    /// @return Error code.
    ///
    /// @param sql SQL statement to execute.
    PreparedStmt::ReturnCode exec(std::string_view sql);

    /// @brief Get a prepared statement for this database connection.
    ///
    /// @return Pointer to a prepared statement.
    ///
    /// \param sql SQL statement to create the prepared statement from.
    PreparedStmt getStmt(std::string const &sql);

private:
    std::shared_ptr<PooledConnection> pooled_conn_; ///< Pooled connection manager.
};
} // namespace dbpool

#endif
