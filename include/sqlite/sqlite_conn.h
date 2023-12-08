#pragma once

#include <memory>
#include <sqlite3.h>
#include <string>
#include <unordered_map>
#include "db_conn.h"

//!
//! \class sqlite_conn
//!
//! \brief Class that implements a connection to an SQLite3 database.
//!
class sqlite_conn
		: public db_conn {
public:
	using stmt_cache_type = std::unordered_map<std::string, sqlite3_stmt*>;

public:
	sqlite_conn(sqlite_conn&& conn);
	sqlite_conn(sqlite3* db);
	~sqlite_conn() override;

	sqlite_conn& operator=(sqlite_conn&& conn);

	sqlite_conn(const sqlite_conn&) = delete;
	sqlite_conn& operator=(const sqlite_conn&) = delete;

	bool is_open() override;
	db_stmt::return_code exec(std::string_view sql) override;
	std::unique_ptr<db_stmt> get_stmt(const std::string& sql) override;

private:
	sqlite3* mDb{};
	stmt_cache_type mStmtCache;
};
