// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#ifndef DBPOOL_CONNECTIONPOOLIMPL_HPP
#define DBPOOL_CONNECTIONPOOLIMPL_HPP

#include "ConnectionImpl.hpp"
#include "dbpool/Connection.hpp"
#include <condition_variable>
#include <forward_list>
#include <memory>

namespace dbpool {
/// @class ConnectionPoolImpl
///
/// @brief Factor class for creates database connection.
///
/// When adding new database types to the library this class needs to be overridden to create the new database
/// \c ConnectionImpl objects that will support connections to the new database.
class ConnectionPoolImpl {
	friend Connection;

public:
	using conn_cache_t = std::forward_list<std::unique_ptr<ConnectionImpl> >;

public:
	ConnectionPoolImpl() = delete;
	ConnectionPoolImpl(ConnectionPoolImpl const&) = delete;
	ConnectionPoolImpl(ConnectionPoolImpl&&) noexcept = delete;

	/// @brief Creates a connection pool with \c pool_size database connections.
	///
	/// @param pool_size Size of the connection pool.
	explicit ConnectionPoolImpl(size_t pool_size);

	virtual ~ConnectionPoolImpl() = default;

	ConnectionPoolImpl& operator=(ConnectionPoolImpl const&) = delete;
	ConnectionPoolImpl& operator=(ConnectionPoolImpl&&) noexcept = delete;

	virtual int64_t getSchema() = 0;
	virtual void setSchema(int64_t schema) = 0;

	/// @brief Pops a concrete database connection class from the queue of available connection objects..
	///
	/// @return Pointer to a concrete database connection object.
	std::unique_ptr<ConnectionImpl> popConnection();

	/// @brief Pushes a concreted database connection back into the connection pool for reuse.
	///
	/// @param conn Database connection to return to the connection pool.
	void pushConnection(std::unique_ptr<ConnectionImpl> conn);

	/// @brief Stores the SQL statement, \c sql, for use when preparing new database connections.
	///
	/// @param sql SQL statement to execute when opening a new database connection.
	///
	/// Some databases (such as SQLite3) require some statements (i.e., statements to create temporary table views) to
	/// be executed on each database connection that is opened in order for each database connection to have a
	/// consistent view of the database. This function sets the statement that you want to have executed on any newly
	/// created database connections that get created by this connection pool.
	void setPrepSql(std::string_view sql);

	/// @brief Returns number of connections available.
	///
	/// @return Number of available connections.
	size_t count();

protected:
	/// @brief Function to create a new database connection object to keep in the connection pool.
	///
	/// @return A pointer to a new \c ConnectionImpl object.
	virtual std::unique_ptr<ConnectionImpl> createConnection() = 0;

private:
	/// @brief Executes the stored SQL preparation statement against the database connection \c conn.
	///
	/// @param conn Database connection to execute the preparation statement against.
	void prepareConnection(ConnectionImpl& conn) const;

	std::condition_variable conn_cv_; ///< Condition variable used when pushing/popping connections from the collection.
	std::mutex conn_mutex_; ///< Locking mutex used to control access to the connection collection.
	conn_cache_t available_conns_; ///< Collection of available connections.

	std::string prep_sql_; ///< SQL statement to execute against newly created database connections.
};
} // namespace dbpool

#endif
