#pragma once

#include <memory>
#include "db_stmt.h"

class db_conn;
class db_conn_pool;

//!
//! \class db_conn_guard
//! \brief Database connection guard.
//!
//! The responsibility of this class is to return the \c db_conn class back to its parent \c db_conn_pool object. This
//! class provides a thin wrapper around the \c db_conn object and will delegate most of its function calls to its
//! \c db_conn member. Upon destruction, objects of this type will return their \c db_conn database handle to the
//! connection pool for reuse.
//!
//! Also, since this object is not responsible for closing database connection, this class uses \c shared_ptr semantics
//! to keep the owning connection pool alive so that the \c db_conn class can be returned to it for reuse/deletion.
//!
class db_conn_guard
		: public std::enable_shared_from_this<db_conn_guard> {
public:
	db_conn_guard(const db_conn_guard&) = delete;
	db_conn_guard(db_conn_guard&&) = delete;
	db_conn_guard(db_conn* connGuard, std::shared_ptr<db_conn_pool> connPool);

	virtual ~db_conn_guard();

	db_conn_guard& operator=(const db_conn_guard&) = delete;
	db_conn_guard& operator=(db_conn_guard&& connGuard) = delete;

	db_stmt::return_code exec(std::string_view sql);
	std::unique_ptr<db_stmt> get_stmt(const std::string& sql);

private:
	db_conn* mConn{};
	std::shared_ptr<db_conn_pool> mConnPool;
};
