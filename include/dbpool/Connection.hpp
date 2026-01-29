#ifndef DBPOOL_CONNECTION_HPP
#define DBPOOL_CONNECTION_HPP

#include <memory>

#include "dbpool/PreparedStmt.hpp"

namespace dbpool {

class ConnectionImpl;
class ConnectionPool;

//!
//! \class Connection
//! \brief Database connection guard.
//!
//! The responsibility of this class is to return the \c db_conn_impl class back to its parent \c db_conn_pool object.
//! This class provides a thin wrapper around the \c db_conn_impl object and will delegate most of its function calls to
//! its //! \c db_conn_impl member. Upon destruction, objects of this type will return their \c db_conn_impl database
//! handle to the connection pool for reuse.
//!
class Connection : public std::enable_shared_from_this<Connection> {
public:
    Connection(Connection const &) = delete;
    Connection(Connection &&) noexcept = default;
    Connection(std::unique_ptr<ConnectionImpl> conn, std::shared_ptr<ConnectionPool> conn_pool);

    virtual ~Connection();

    Connection &operator=(Connection const &) = delete;
    Connection &operator=(Connection &&) noexcept = default;

    PreparedStmt::return_code exec(std::string_view sql);
    std::unique_ptr<PreparedStmt> get_stmt(std::string const &sql);

private:
    std::unique_ptr<ConnectionImpl> m_conn;
    std::shared_ptr<ConnectionPool> m_conn_pool;
};

} // namespace dbpool

#endif
