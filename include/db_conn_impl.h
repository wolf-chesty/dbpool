#pragma once

#include <memory>

#include "db_conn.h"
#include "db_stmt.h"

namespace dbpool {

//!
//! \class db_conn_impl
//! \brief Database connection class.
//!
//! This class abstracts away the nuances of the various database connection API's so that the dependent application can
//! use a single interface when executing SQL statements.
//!
class db_conn_impl {
public:
    db_conn_impl() = default;
    db_conn_impl(db_conn_impl const &) = delete;
    db_conn_impl(db_conn_impl &&) = delete;

    virtual ~db_conn_impl() = default;

    db_conn_impl &operator=(db_conn_impl const &) = delete;
    db_conn_impl &operator=(db_conn_impl &&) = delete;

    //!
    //! \brief Executes an SQL statements directly on this database connection.
    //!
    //! \param sql SQL statement to execute.
    //! \return \c db_stmt::return_code.
    //!
    //! Executes an SQL statement directly on the database connection without creating a prepared statement.
    //!
    virtual db_stmt::return_code exec(std::string_view sql) = 0;

    //!
    //! \brief Get a prepared statement for this database connection.
    //!
    //! \param conn \c db_conn that owns the prepared statement.
    //! \param sql SQL statement to create the prepared statement from.
    //! \return Pointer to a prepared statement object.
    //!
    //! Implementers of this function should return a handle to the a prepared statement that can be used with this
    //! database connection.
    //!
    virtual std::unique_ptr<db_stmt> get_stmt(std::shared_ptr<db_conn> conn, std::string const &sql) = 0;
};

} // namespace dbpool