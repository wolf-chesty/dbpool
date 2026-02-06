// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#ifndef DBPOOL_CONNECTION_HPP
#define DBPOOL_CONNECTION_HPP

#include <memory>

#include "dbpool/PreparedStmt.hpp"

namespace dbpool {

class ConnectionImpl;
class ConnectionPoolImpl;

///
/// @class Connection
///
/// @brief This class wraps the concrete implementations of \c ConnectionImpl and will return the implementation back to
///        the managing \c ConnectionPool.
///
/// This object manages the concrete database connection implementations and is responsible for returning unused
/// database connections to the connection pool object that created them. This object delegates all database
/// functionality to the underlying concrete \c ConnectionImpl class which implements any API specific database code.
///
class Connection : public std::enable_shared_from_this<Connection> {
public:
    Connection() = delete;
    Connection(Connection const &) = delete;
    Connection(Connection &&) noexcept = default;

    /// @brief ctor for the object.
    ///
    /// @param conn Database connection.
    /// @param conn_pool Connection pool that owns the database connection \c conn.
    explicit Connection(std::unique_ptr<ConnectionImpl> conn, std::shared_ptr<ConnectionPoolImpl> conn_pool);

    /// @brief dtor for the object.
    ///
    /// Returns the managed connection object to its connection pool.
    virtual ~Connection();

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
    std::unique_ptr<PreparedStmt> get_stmt(std::string const &sql);

private:
    std::unique_ptr<ConnectionImpl> conn_;          ///< API specific database connection implementation.
    std::shared_ptr<ConnectionPoolImpl> conn_pool_; ///< Connection pool that created the managed connection object.
};

} // namespace dbpool

#endif
