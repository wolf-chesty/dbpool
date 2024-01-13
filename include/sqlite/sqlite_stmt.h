#pragma once

#include <sqlite3.h>
#include "db_stmt.h"

class sqlite_stmt
		: public db_stmt {
public:
	sqlite_stmt(sqlite_stmt const&) = delete;
	sqlite_stmt(sqlite_stmt&&) = delete;
	sqlite_stmt(std::shared_ptr<db_conn_guard> conn, sqlite3* db, sqlite3_stmt* stmt);

	~sqlite_stmt() override;

	sqlite_stmt& operator=(sqlite_stmt const&) = delete;
	sqlite_stmt& operator=(sqlite_stmt&&) = delete;

	return_code execute() override;

	void bind_blob(int32_t const index, void const* data, size_t const nbytes) override;
	void bind_bool(int32_t const index, bool const value) override;
	void bind_date(int32_t const index, std::string_view value) override;
	void bind_double(int32_t const index, double const value) override;
	void bind_int32(int32_t const index, int32_t const value) override;
	void bind_int64(int32_t const index, int64_t const value) override;
	void bind_null(int32_t const index) override;
//	void bind_uuid(int32_t const index, uuids::uuid const& id) override;
//	void bind_uuid(int32_t const index, uuids::uuid&& id) override;
	void bind_text(int32_t const index, std::string_view text) override;

	bool get_bool(int32_t const index) override;
	std::string get_date(int32_t const index) override;
	double get_double(int32_t const index) override;
	int32_t get_int32(int32_t const index) override;
	int64_t get_int64(int32_t const index) override;
	std::string get_text(int32_t const index) override;
//	uuids::uuid get_uuid(int32_t const index) override;

	static return_code to_error_code(int code);

private:
	sqlite3* mDb{};
	sqlite3_stmt* mStmt{};
};
