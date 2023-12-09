#pragma once

#include <memory>
#include "db_conn_guard.h"
#include "db_stmt.h"

//!
//! \class db_conn
//! \brief Database connection class.
//!
//! The responsibility of this class is to abstract away the nuances of the various database connection API's so that
//! this library can use a single interface when executing SQL statements.
//!
//! Consumers of this library will never see an object of this type as it's only used by the \c db_conn_guard object
//! when communicating with the database.
//!
class db_conn {
public:
	db_conn() = default;
	db_conn(const db_conn&) = delete;
	db_conn(db_conn&&) = delete;

	virtual ~db_conn() = default;

	db_conn& operator=(const db_conn&) = delete;
	db_conn& operator=(db_conn&&) = delete;

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
	//! \param conn \c db_conn_guard that owns the prepared statement.
	//! \param sql SQL statement to create the prepared statement from.
	//! \return Pointer to a prepared statement object.
	//!
	//! Implementers of this function should return a handle to the a prepared statement that can be used with this
	//! database connection.
	//!
	virtual std::unique_ptr<db_stmt> get_stmt(std::shared_ptr<db_conn_guard> conn, const std::string& sql) = 0;
};
