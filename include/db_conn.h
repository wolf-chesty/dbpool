#pragma once

#include <memory>
#include "db_stmt.h"

//!
//! \class db_conn
//! \brief Database connection class.
//!
//! This class provides an interface for a connection to a database.
//!
class db_conn {
public:
	db_conn() = default;
	virtual ~db_conn() = default;

	db_conn(const db_conn&) = delete;
	db_conn& operator=(const db_conn&) = delete;

    //!
    //! \brief Returns \c true if the database is open.
    //!
    //! \return \c true if the database is open.
    //!
	virtual bool is_open() = 0;

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
    //! \param sql SQL statement to create the prepared statement from.
    //! \return Pointer to a prepared statement object.
    //!
    //! Implementers of this function should return a handle to the a prepared statement that can be used with this
    //! database connection.
    //!
	virtual std::unique_ptr<db_stmt> get_stmt(const std::string& sql) = 0;
};
