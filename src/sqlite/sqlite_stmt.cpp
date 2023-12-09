#include <cassert>
#include <chrono>
#include <cstring>
#include <date/date.h>
#include <fmt/core.h>
#include <iostream>
#include <sstream>
#include "sqlite/sqlite_stmt.h"

//!
//! \brief Constructs a prepared statement for connection \c db using SQL statement \c stmt.
//!
//! \param db Database connection that this prepared statement is bound to.
//! \param stmt SQL statement to create prepared statement from.
//!
sqlite_stmt::sqlite_stmt(std::shared_ptr<db_conn_guard> conn, sqlite3* db, sqlite3_stmt* stmt)
		:db_stmt(conn), mDb(db), mStmt(stmt)
{
}

//!
//! \brief Resets the underlying prepared statement for reuse.
//!
//! Prepared statements are cached for reuse so rather than actually freeing the prepared statement memory this function
//! will unbind and reset the prepared statement to its initial state for reuse by another process. Prepared statement
//! deletion will actually be performed by the database connection that created it.
//!
sqlite_stmt::~sqlite_stmt()
{
	assert(mStmt);
	if (sqlite3_bind_parameter_count(mStmt))
		sqlite3_clear_bindings(mStmt);
	sqlite3_reset(mStmt);
}

sqlite_stmt::return_code sqlite_stmt::to_error_code(int code)
{
	switch (code) {
	case SQLITE_OK: return return_code::ok;
	case SQLITE_ROW: return return_code::row;
	case SQLITE_DONE: return return_code::done;
	}
	return return_code::error;
}

sqlite_stmt::return_code sqlite_stmt::execute()
{
	return to_error_code(sqlite3_step(mStmt));
}

void sqlite_stmt::bind_blob(const int32_t index, const void* data, const size_t nbytes)
{
	if (sqlite3_bind_blob(mStmt, index, data, nbytes, SQLITE_TRANSIENT))
		throw std::runtime_error(fmt::format("sqlite_stmt::bind_blob: {}", sqlite3_errmsg(mDb)));
}

void sqlite_stmt::bind_bool(const int32_t index, const bool value)
{
	if (sqlite3_bind_int(mStmt, index, value ? 1 : 0))
		throw std::runtime_error(fmt::format("sqlite_stmt::bind_bool: {}", sqlite3_errmsg(mDb)));
}

void sqlite_stmt::bind_date(const int32_t index, std::string_view value)
{
	int ret{};
	if (value.empty()) {
		ret = sqlite3_bind_null(mStmt, index);
	}
	else {
		// TODO:
		//  We need to evaluate this code; according to sources on the internet parsing dates using the standard
		// 	template library can be slower than parsing the JSON data. Perhaps switching to a database with native ISO
		// 	8601 date support would allow us to offload date parsing to another process? In the mean time, since parsing
		// 	happens on a separate thread we can safely assume that the web interface isn't hung up waiting for date
		// 	parsing to complete.

		// parse ISO 8601 date
		const std::string v(value);
		std::istringstream ss{v};
		date::sys_time<std::chrono::milliseconds> t;
		ss >> date::parse("%FT%TZ", t);            // timestamp without TZ offset
		if (ss.fail()) {
			// failed to parse the date; perhaps Flowroute sent a timestampe
			// with TZ offset, try again
			ss.clear();
			ss.str(v);
			ss >> date::parse("%FT%T%Ez", t);    // timestamp with TZ offset
		}

		// place number of milliseconds since epoch into database
		const int64_t millis = t.time_since_epoch().count();
		ret = sqlite3_bind_int(mStmt, index, millis);
	}

	if (ret)
		throw std::runtime_error(fmt::format("sqlite_stmt::bind_date: {}", sqlite3_errmsg(mDb)));
}

void sqlite_stmt::bind_double(const int32_t index, const double value)
{
	if (sqlite3_bind_double(mStmt, index, value))
		throw std::runtime_error(fmt::format("sqlite_stmt::bind_double: {}", sqlite3_errmsg(mDb)));
}

void sqlite_stmt::bind_int32(const int32_t index, const int32_t value)
{
	if (sqlite3_bind_int(mStmt, index, value))
		throw std::runtime_error(fmt::format("sqlite_stmt::bind_int32: {}", sqlite3_errmsg(mDb)));
}

void sqlite_stmt::bind_int64(const int32_t index, const int64_t value)
{
	if (sqlite3_bind_int64(mStmt, index, value))
		throw std::runtime_error(fmt::format("sqlite_stmt::bind_int64: {}", sqlite3_errmsg(mDb)));
}

void sqlite_stmt::bind_null(const int32_t index)
{
	if (sqlite3_bind_null(mStmt, index))
		throw std::runtime_error(fmt::format("sqlite_stmt::bind_null: {}", sqlite3_errmsg(mDb)));
}

/*
void sqlite_stmt::bind_uuid(const int32_t index, const uuids::uuid& value)
{
	auto d = value.as_bytes();
	if (sqlite3_bind_blob(mStmt, index, &d[0], d.size(), SQLITE_TRANSIENT)) {
		const auto e = fmt::format("sqlite_stmt::bind_blob: {}", sqlite3_errmsg(mDb));
		throw std::runtime_error(e);
	}
}

void sqlite_stmt::bind_uuid(const int32_t index, uuids::uuid&& value)
{
	bind_uuid(index, value);
}
*/

void sqlite_stmt::bind_text(const int32_t index, std::string_view value)
{
	const int ret = value.empty()
					? sqlite3_bind_null(mStmt, index)
					: sqlite3_bind_text(mStmt, index, value.data(), static_cast<int>(value.length()), SQLITE_TRANSIENT);

	if (ret)
		throw std::runtime_error(fmt::format("sqlite::bind_text: {}", sqlite3_errmsg(mDb)));
}

bool sqlite_stmt::get_bool(const int32_t index)
{
	return sqlite3_column_int(mStmt, index) != 0;
}

std::string sqlite_stmt::get_date(const int32_t index)
{
	// sqlite doesn't have a native date field; keep number of milliseconds since epoch in database
	auto millis = sqlite3_column_int64(mStmt, index);
	// format timestamp
	std::chrono::milliseconds d{millis};
	date::sys_time<std::chrono::milliseconds> tp{d};
	return date::format("%FT%TZ", date::floor<std::chrono::milliseconds>(tp));
}

double sqlite_stmt::get_double(const int32_t index)
{
	return sqlite3_column_double(mStmt, index);
}

int32_t sqlite_stmt::get_int32(const int32_t index)
{
	return sqlite3_column_int(mStmt, index);
}

int64_t sqlite_stmt::get_int64(const int32_t index)
{
	return sqlite3_column_int64(mStmt, index);
}

std::string sqlite_stmt::get_text(const int32_t index)
{
	return reinterpret_cast<const char*>(sqlite3_column_text(mStmt, index));
}

/*
uuids::uuid sqlite_stmt::get_uuid(const int32_t index)
{
	uuids::uuid id;

	// the following is janky as hell; in order to avoid a crazy amount of copies we are going to cast away the const
 	// and overwrite the uuid in place
	if (auto blob = sqlite3_column_blob(mStmt, index)) {
		auto d = id.as_bytes();
		memcpy((void*)&d[0], blob, d.size());
	}
	return id;
}
*/