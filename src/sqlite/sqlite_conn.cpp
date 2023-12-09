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
    close();
}

//!
//! \brief Takes ownership of the \c sqlite3 connection from the \c conn connection.
//!
//! \param conn \c sqlite_conn to take ownership from.
//! \return Reference to this object.
//!
sqlite_conn& sqlite_conn::operator=(sqlite_conn&& conn)
{
    close();

	mDb = conn.mDb;
	conn.mDb = nullptr;

    // do we really need to keep the statement cache?
	mStmtCache = std::move(conn.mStmtCache);

	return *this;
}

//!
//! \brief Closes the database connection and frees the memory associated with the connection.
//!
void sqlite_conn::close()
{
    // make sure to clean up memory for cached prepared statements
    for (auto& stmt : mStmtCache) {
        sqlite3_finalize(stmt.second);
    }

    // close database handle
    assert(mDb);
    sqlite3_close(mDb);
}

//!
//! \brief Returns true if the database connection is open.
//!
//! \return \c true if the database connection is open.
//!
bool sqlite_conn::is_open()
{
	return mDb != nullptr;
}

//!
//! \brief Executes statement \c sql on the database connection.
//!
//! \param sql SQL statement to execute.
//! \return Return code for statement executed.
//!
db_stmt::return_code sqlite_conn::exec(std::string_view sql)
{
	assert(mDb);
	return sqlite_stmt::to_error_code(sqlite3_exec(mDb, sql.data(), nullptr, nullptr, nullptr));
}

//!
//! \brief Returns a prepared statement for this database connection.
//!
//! \param conn Connection guard that initiated the construction of the prepared statement.
//! \param sql SQL statement of the prepared statement.
//! \return Pointer to a prepared statement.
//!
std::unique_ptr<db_stmt> sqlite_conn::get_stmt(std::shared_ptr<db_conn_guard> conn, const std::string& sql)
{
	assert(mDb);

	std::unique_ptr<db_stmt> p;
	auto it = mStmtCache.find(sql);
	if (it == mStmtCache.end()) {
		// create new stored procedure for connection
		sqlite3_stmt* stmt;
		if (sqlite3_prepare_v2(mDb, sql.data(), sql.length() + 1, &stmt, nullptr))
			throw std::runtime_error(fmt::format("sqlite_conn::get_stmt: {}", sqlite3_errmsg(mDb)));

		mStmtCache.emplace(std::make_pair(sql, stmt));
		p.reset(new sqlite_stmt(conn, mDb, stmt));
	}
	else {
		p.reset(new sqlite_stmt(conn, mDb, it->second));
	}
	return p;
}
