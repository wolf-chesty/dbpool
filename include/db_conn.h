#pragma once

#include <memory>

#include "db_stmt.h"

class db_conn_impl;
class db_conn_pool;

//!
//! \class db_conn
//! \brief Database connection guard.
//!
//! The responsibility of this class is to return the \c db_conn_impl class back to its parent \c db_conn_pool object.
//! This class provides a thin wrapper around the \c db_conn_impl object and will delegate most of its function calls to
//! its //! \c db_conn_impl member. Upon destruction, objects of this type will return their \c db_conn_impl database
//! handle to the connection pool for reuse.
//!
class db_conn : public std::enable_shared_from_this<db_conn> {
public:
    db_conn(db_conn const &) = delete;
    db_conn(db_conn &&) = delete;
    db_conn(std::unique_ptr<db_conn_impl> conn, std::shared_ptr<db_conn_pool> connPool);

    virtual ~db_conn();

    db_conn &operator=(db_conn const &) = delete;
    db_conn &operator=(db_conn &&) = delete;

    db_stmt::return_code exec(std::string_view sql);
    std::unique_ptr<db_stmt> get_stmt(std::string const &sql);

private:
    std::unique_ptr<db_conn_impl> mConn;
    std::shared_ptr<db_conn_pool> mConnPool;
};
