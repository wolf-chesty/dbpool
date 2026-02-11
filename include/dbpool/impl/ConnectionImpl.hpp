// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#ifndef DBPOOL_CONNECTION_IMPL_HPP
#define DBPOOL_CONNECTION_IMPL_HPP

#include "PreparedStmtImpl.hpp"
#include <memory>
#include <string>

namespace dbpool {

///
/// @class ConnectionImpl
///
/// @brief Database connection class.
///
/// This class abstracts away the nuances of the various database connection API's so that the dependent application can
/// use a single interface when executing SQL statements.
///
class ConnectionImpl {
public:
    ConnectionImpl() = default;
    ConnectionImpl(ConnectionImpl const &) = delete;
    ConnectionImpl(ConnectionImpl &&) noexcept = delete;
    virtual ~ConnectionImpl() = default;

    ConnectionImpl &operator=(ConnectionImpl const &) = delete;
    ConnectionImpl &operator=(ConnectionImpl &&) noexcept = delete;

    /// @brief Executes an SQL statements directly on this database connection.
    ///
    /// @return \c db_stmt::return_code.
    ///
    /// @param sql SQL statement to execute.
    ///
    /// Executes an SQL statement directly on the database connection without creating a prepared statement.
    virtual PreparedStmtImpl::ReturnCode exec(std::string_view sql) = 0;

    /// @brief Get a prepared statement for this database connection.
    ///
    /// @return Pointer to a prepared statement object.
    ///
    /// @param sql SQL statement to create the prepared statement from.
    ///
    /// Implementers of this function should return a pointer to a prepared statement that can be used with this
    /// database connection.
    virtual std::unique_ptr<PreparedStmtImpl> get_stmt(std::string const &sql) = 0;
};

} // namespace dbpool

#endif
