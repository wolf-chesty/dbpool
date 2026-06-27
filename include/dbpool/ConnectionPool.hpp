// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#ifndef DBPOOL_CONNECTION_POOL_HPP
#define DBPOOL_CONNECTION_POOL_HPP

#include "dbpool/ConnectionPoolBase.hpp"

#include "dbpool/Connection.hpp"
#include "dbpool/util/PooledConnection.hpp"
#include "impl/ConnectionPoolImpl.hpp"
#include <cassert>
#include <memory>
#include <string_view>

namespace dbpool {

class ConnectionImpl;

template<class T>
concept ConnectionPoolImplT = std::derived_from<T, ConnectionPoolImpl>;

/// @class ConnectionPool
///
/// @brief A database connection pool class.
///
/// The responsibility of this class is to manage (create and destroy) \c db_conn objects that will be used by the
/// application to communicate with the database. This class will return \c db_conn objects to consumers of this
/// library.
///
/// Classes that derive from this will need to implement the API specific code required to return a \c db_conn object
/// that wraps a API specific database handle.
template<ConnectionPoolImplT connection_pool_t> class ConnectionPool : public ConnectionPoolBase {
public:
    ConnectionPool(ConnectionPool const &) = delete;
    ConnectionPool(ConnectionPool &&) noexcept = default;

    explicit ConnectionPool(std::shared_ptr<connection_pool_t> conn_pool)
        : conn_pool_(std::move(conn_pool))
    {
    }

    virtual ~ConnectionPool() = default;

    ConnectionPool &operator=(ConnectionPool const &) = delete;
    ConnectionPool &operator=(ConnectionPool &&) noexcept = default;

    /// @brief Returns the schema number of the database.
    ///
    /// @return Database schema number.
    int64_t getSchema()
    {
        assert(conn_pool_);
        return conn_pool_->getSchema();
    }

    /// @brief Sets the schema number for the database.
    ///
    /// @param schema Database schema number.
    void setSchema(int64_t schema)
    {
        assert(conn_pool_);
        conn_pool_->setSchema(schema);
    }

    /// @brief Returns a new \c Connection object that can be used to execute statements against the database.
    ///
    /// @return Pointer to a new \c Connection object.
    Connection getConnection() override
    {
        assert(conn_pool_);
        return Connection(std::make_shared<PooledConnection>(conn_pool_, conn_pool_->popConnection()));
    }

    /// @brief Stores the SQL statement, \c sql, for use when preparing new database connections.
    ///
    /// @param sql SQL statement to execute when opening a new database connection.
    ///
    /// Some databases (such as SQLite3) require some statements (i.e., statements to create temporary table views) to
    /// be executed on each database connection that is opened in order for each database connection to have a
    /// consistent view of the database. This function sets the statement that you want to have executed on any newly
    /// created database connections that get created by this connection pool.
    void setPrepSql(std::string_view sql)
    {
        assert(conn_pool_);
        conn_pool_->setPrepSql(sql);
    }

    /// @brief Returns number of connections available in pool.
    ///
    /// @return Number of connections available in pool.
    size_t count()
    {
        assert(conn_pool_);
        return conn_pool_->count();
    }

protected:
    /// @brief Returns pointer to the connection pool implementation.
    ///
    /// @return Pointer to connection pool implementation.
    connection_pool_t *getConnectionPool() const
    {
        return conn_pool_.get();
    }

private:
    std::shared_ptr<connection_pool_t> conn_pool_; ///< Pointer to connection pool implementation.
};

} // namespace dbpool

#endif
