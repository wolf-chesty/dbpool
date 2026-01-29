#ifndef DBPOOL_SQLITE_CONNECTION_IMPL_HPP
#define DBPOOL_SQLITE_CONNECTION_IMPL_HPP

#include "dbpool/ConnectionImpl.hpp"

#include <map>
#include <memory>
#include <sqlite3.h>
#include <string>

namespace dbpool::sqlite {

//!
//! \class ConnectionImpl
//! \brief Class that implements a connection to an SQLite3 database.
//!
class ConnectionImpl : public dbpool::ConnectionImpl {
public:
    using stmt_cache_type = std::map<std::string, sqlite3_stmt *>;

public:
    ConnectionImpl(ConnectionImpl const &) = delete;
    ConnectionImpl(ConnectionImpl &&) noexcept = delete;
    explicit ConnectionImpl(sqlite3 *db);
    ~ConnectionImpl() override;

    ConnectionImpl &operator=(ConnectionImpl const &) = delete;
    ConnectionImpl &operator=(ConnectionImpl &&conn) noexcept = delete;

    dbpool::PreparedStmt::return_code exec(std::string_view sql) override;
    std::unique_ptr<PreparedStmt> get_stmt(std::shared_ptr<dbpool::Connection> conn, std::string const &sql) override;

private:
    sqlite3 *m_db{};
    stmt_cache_type m_stmt_cache;
};

} // namespace dbpool::sqlite

#endif
