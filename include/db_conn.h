#pragma once

#include <memory>
#include "db_stmt.h"

class db_conn {
public:
	db_conn() = default;
	virtual ~db_conn() = default;

	db_conn(const db_conn&) = delete;
	db_conn& operator=(const db_conn&) = delete;

	virtual bool is_open() = 0;

	virtual db_stmt::return_code exec(std::string_view sql) = 0;
	virtual std::unique_ptr<db_stmt> get_stmt(const std::string& sql) = 0;
};
