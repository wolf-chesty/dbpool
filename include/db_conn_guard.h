#pragma once

#include <memory>
#include "db_stmt.h"

class db_conn;
class db_conn_pool;

//!
//! \class db_conn_guard
//! \brief Database connection guard.
//!
//! This class maintains the lifetime of the \c db_conn class and is responsible for adding the connection back to the
//! connection pool whenever the object goes out of scope.
//!
class db_conn_guard {
public:
    db_conn_guard(const db_conn_guard&) = delete;
    db_conn_guard(db_conn_guard&& connGuard);
	db_conn_guard(db_conn* connGuard, std::shared_ptr<db_conn_pool> connPool);
	virtual ~db_conn_guard();

    db_conn_guard& operator=(const db_conn_guard&) = delete;
	db_conn_guard& operator=(db_conn_guard&& connGuard);

	db_stmt::return_code exec(std::string_view sql);
	std::unique_ptr<db_stmt> get_stmt(const std::string& sql);

private:
	db_conn* mConn;
	std::shared_ptr<db_conn_pool> mConnPool;
};
