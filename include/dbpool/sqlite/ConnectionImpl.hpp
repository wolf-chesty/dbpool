// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#ifndef DBPOOL_SQLITE_CONNECTION_IMPL_HPP
#define DBPOOL_SQLITE_CONNECTION_IMPL_HPP

#include "dbpool/impl/ConnectionImpl.hpp"

#include <memory>
#include <sqlite3.h>

namespace dbpool::sqlite {
/// @class ConnectionImpl
///
/// @brief Class that implements a connection to an SQLite3 database.
///
/// This class controls the lifetime of a connection to an SQLite3 database. Upon destruction, the object will close the
/// database.
///
/// The object also maintains a cache of prepared statements in order to minimize the re-compilation of frequently used
/// prepared statements.
class ConnectionImpl : public dbpool::ConnectionImpl {
public:
	ConnectionImpl(ConnectionImpl const&) = delete;
	ConnectionImpl(ConnectionImpl&&) noexcept = delete;

	/// @brief ctor for object.
    ///
    /// @param db SQLite3 database connection.
    ///
    /// This object takes ownership of the sqlite3 database pointer, \c db.
	explicit ConnectionImpl(sqlite3* db);

	/// @brief dtor for object.
    ///
    /// Releases cached prepared statements and closes the SQLite3 database.
	~ConnectionImpl() override;

	ConnectionImpl& operator=(ConnectionImpl const&) = delete;
	ConnectionImpl& operator=(ConnectionImpl&& conn) noexcept = delete;

	/// @brief Executes statement \c sql on the managed database connection.
    ///
    /// @return Return code for statement executed.
    ///
    /// @param sql SQL statement to execute.
    ///
    /// This function will execute the SQL statement \c sql on the managed database connection, bypassing the creation
    /// of a prepared statement. The use of this function is ideal for one-off, infrequently executed SQL statements.
	PreparedStmtImpl::ReturnCode exec(std::string_view sql) override;

	/// @brief Returns a prepared statement for this object's managed database connection.
    ///
    /// @return Pointer to a prepared statement.
    ///
    /// @param sql SQL statement of the prepared statement.
    ///
    /// Searches the object's collection of cached prepared statements for the SQL statement \c sql. If the prepared
    /// statement exists it is returned for re-use. If not, a new prepared statement is created and then cached using
    /// the SQL statement \c sql as the statement key.
	std::unique_ptr<dbpool::PreparedStmtImpl> getStmt(std::string const& sql) override;

private:
	sqlite3* db_{}; ///< Pointer to SQLite3 database.
};
} // namespace dbpool::sqlite

#endif
