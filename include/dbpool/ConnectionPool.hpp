// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#ifndef DBPOOL_CONNECTION_POOL_HPP
#define DBPOOL_CONNECTION_POOL_HPP

#include "dbpool/Connection.hpp"
#include <condition_variable>
#include <forward_list>
#include <memory>
#include <mutex>
#include <string>

namespace dbpool {

class ConnectionImpl;

//!
//! \class ConnectionPool
//! \brief A database connection pool class.
//!
//! The responsibility of this class is to manage (create and destroy) \c db_conn objects that will
//! be used by the application to communicate with the database. This class will return \c
//! db_conn objects to consumers of this library.
//!
//! Classes that derive from this will need to implement the API specific code required to return a
//! \c db_conn object that wraps a API specific database handle.
//!
class ConnectionPool {
    friend Connection;

public:
    using conn_cache_t = std::forward_list<std::unique_ptr<ConnectionImpl>>;

public:
    ConnectionPool(ConnectionPool const &) = delete;
    ConnectionPool(ConnectionPool &&) noexcept = delete;

    /// @brief Creates a connection pool with \c pool_size database connections.
    ///
    /// @param pool_size Size of the connection pool.
    explicit ConnectionPool(size_t pool_size);

    virtual ~ConnectionPool() = default;

    ConnectionPool &operator=(ConnectionPool const &) = delete;
    ConnectionPool &operator=(ConnectionPool &&) noexcept = delete;

    /// @brief Returns a pointer to a new \c Connection object that can be used to execute statements against the
    ///        database.
    ///
    /// @return Pointer to a new \c Connection object.
    std::shared_ptr<Connection> get_conn();

    virtual int64_t get_schema() = 0;
    virtual void set_schema(int64_t schema) = 0;

    /// @brief Stores the SQL statement, \c sql, for use when preparing new database connections.
    ///
    /// @param sql SQL statement to execute when opening a new database connection.
    ///
    /// Some databases (such as SQLite3) require some statements (i.e., statements to create temporary table views) to
    /// be executed on each database connection that is opened in order for each database connection to have a
    /// consistent view of the database. This function sets the statement that you want to have executed on any newly
    /// created database connections that get created by this connection pool.
    void set_prep_sql(std::string_view sql);

protected:
    /// @brief Pops a concrete database connection class from the queue of available connection objects..
    ///
    /// @return Pointer to a concrete database connection object.
    std::unique_ptr<ConnectionImpl> pop_conn();

    /// @brief Pushes a concreted database connection back into the connection pool for reuse.
    ///
    /// @param conn Database connection to return to the connection pool.
    void push_conn(std::unique_ptr<ConnectionImpl> conn);

    /// @brief Function to create a new database connection object to keep in the connection pool.
    ///
    /// @return A pointer to a new \c ConnectionImpl object..
    virtual std::unique_ptr<ConnectionImpl> new_conn() = 0;

    /// @brief Returns a \c std::shared_ptr to this object.
    ///
    /// @return \c std::shared_ptr to this object.
    ///
    /// Objects in this library will use managed pointers to their owning connection pool object in order to avoid
    /// prematurely closing/finalizing database connections/libraries. Rather than each object having to have knowledge
    /// of the derived classes type, implementors of this function should return a casted pointer to themselves.
    virtual std::shared_ptr<ConnectionPool> shared_base_ptr() = 0;

    /// @brief Executes the stored SQL preparation statement against the database connection \c conn.
    ///
    /// \param conn Database connection to execute the preparation statement against.
    virtual void prep_conn(ConnectionImpl &);

private:
    std::condition_variable conn_cv_;   ///< Condition variable used when pushing/popping connections from the collection.
    std::mutex conn_mutex_;             ///< Locking mutex used to control access to the connection collection.
    conn_cache_t available_conns_;      ///< Collection of available connections.

    std::string prep_sql_;              ///< SQL statement to execute against newly created database connections.
};

} // namespace dbpool

#endif
