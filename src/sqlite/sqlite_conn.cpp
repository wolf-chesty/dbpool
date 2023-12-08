#include <cassert>
#include <fmt/core.h>
#include "sqlite/sqlite_conn.h"
#include "sqlite/sqlite_stmt.h"

//!
//! \brief Creates an SQLite3 database connection using resources from SQLite3 dtabase connection \c conn.
//!
//! \param conn Database connection to take resources from.
//!
sqlite_conn::sqlite_conn(sqlite_conn&& conn)
{
	mDb = conn.mDb;
	conn.mDb = nullptr;

	mStmtCache = std::move(conn.mStmtCache);
}

//!
//! \brief Creates an \c sqlite_conn wrapper around an \c sqlite3 connection pointer.
//!
//! \param db SQLite3 API database connection.
//!
sqlite_conn::sqlite_conn(sqlite3* db)
		:mDb(db)
{
}

//!
//! \brief Destroys the \c sqlite_conn connection class and resets any cached compiled statements.
//!
sqlite_conn::~sqlite_conn()
{
	assert(mDb);

	// deallocate the SQLite prepared statements
	for (auto& stmt : mStmtCache) {
		sqlite3_finalize(stmt.second);
	}
	// close the database handle
	sqlite3_close(mDb);
}

sqlite_conn& sqlite_conn::operator=(sqlite_conn&& conn)
{
	// make sure to clean up memory from previous connection
	for (auto& stmt : mStmtCache) {
		sqlite3_finalize(stmt.second);
	}
	sqlite3_close

	mDb = conn.mDb;
	conn.mDb = nullptr;

	mStmtCache = std::move(conn.mStmtCache);

	return *this;
}

bool sqlite_conn::is_open()
{
	return mDb != nullptr;
}

db_stmt::return_code sqlite_conn::exec(std::string_view sql)
{
	assert(mDb);
	return sqlite_stmt::to_error_code(sqlite3_exec(mDb, sql.data(), nullptr, nullptr, nullptr));
}

std::unique_ptr<db_stmt> sqlite_conn::get_stmt(const std::string& sql)
{
	assert(mDb);

	std::unique_ptr<db_stmt> p;
	auto it = mStmtCache.find(sql);
	if (it == mStmtCache.end()) {
		// create new stored procedure for connection
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(mDb, sql.data(), sql.length() + 1, &stmt, nullptr)) {
			const auto err = fmt::format("sqlite_conn::get_stmt: {}", sqlite3_errmsg(mDb));
			throw std::runtime_error(err);
		}
		mStmtCache.emplace(std::make_pair(sql, stmt));
		p.reset(new sqlite_stmt(mDb, stmt));
	}
	else {
		p.reset(new sqlite_stmt(mDb, it->second));
	}
	return p;
}
