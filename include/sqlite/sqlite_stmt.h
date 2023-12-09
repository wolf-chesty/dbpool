#pragma once

#include <sqlite3.h>
#include "db_stmt.h"

class sqlite_stmt
		: public db_stmt {
public:
	sqlite_stmt(sqlite_stmt&& stmt);
	sqlite_stmt(std::shared_ptr<db_conn_guard> conn, sqlite3* db, sqlite3_stmt* stmt);
	~sqlite_stmt() override;

	sqlite_stmt& operator=(sqlite_stmt&& stmt);

	sqlite_stmt(const sqlite_stmt&) = delete;
	sqlite_stmt& operator=(const sqlite_stmt&) = delete;

	return_code execute() override;

	void bind_blob(const int32_t index, const void* data, const size_t nbytes) override;
	void bind_bool(const int32_t index, const bool value) override;
	void bind_date(const int32_t index, std::string_view value) override;
	void bind_double(const int32_t index, const double value) override;
	void bind_int32(const int32_t index, const int32_t value) override;
	void bind_int64(const int32_t index, const int64_t value) override;
	void bind_null(const int32_t index) override;
//	void bind_uuid(const int32_t index, const uuids::uuid& id) override;
//	void bind_uuid(const int32_t index, uuids::uuid&& id) override;
	void bind_text(const int32_t index, std::string_view text) override;

	bool get_bool(const int32_t index) override;
	std::string get_date(const int32_t index) override;
	double get_double(const int32_t index) override;
	int32_t get_int32(const int32_t index) override;
	int64_t get_int64(const int32_t index) override;
	std::string get_text(const int32_t index) override;
//	uuids::uuid get_uuid(const int32_t index) override;

	static return_code to_error_code(int code);

private:
	void reset();

	sqlite3* mDb{};
	sqlite3_stmt* mStmt{};
};
