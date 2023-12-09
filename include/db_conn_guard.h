#pragma once

#include <memory>
#include "db_stmt.h"

class db_conn;
class db_conn_pool;

//!
//! \class db_conn_guard
//!
//! \brief Database connection guard.
//!
//! This class maintains the lifetime of a \c db_conn object and is responsible for placing the connection back into its
//! connection pool whenever this object goes out of scope.
//!
//! Concurrent threads should NOT share a \c db_conn_guard class as that can potentially cause crashes and/or invalid
//! indices when multiple threads of execution are returning query results. Each thread should have their own
//! \c db_conn_guard and destroy it when the thread has completed its operation. By persisting this object, other
//! threads can be starved of database connections.
//!
class db_conn_guard
		:std::enable_shared_from_this<db_conn_guard> {
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
	db_conn* mConn{};
	std::shared_ptr<db_conn_pool> mConnPool;
};
