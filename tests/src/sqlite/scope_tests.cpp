// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include "dbpool/PreparedStmt.hpp"
#include "dbpool/sqlite/ConnectionPool.hpp"

TEST(SQLite3Test, connection_guard_scope_test)
{
    dbpool::Connection conn;
    {
        dbpool::sqlite::ConnectionPool db_pool(":memory:");
        conn = db_pool.getConnection();
    }

    // make sure that conn is not invalid once the connection pool shared pointer has died
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::ok, conn.exec("CREATE TABLE t1(x INT)"));
}

TEST(SQLite3Test, statement_scope_test)
{
    dbpool::PreparedStmt stmt;
    {
        dbpool::sqlite::ConnectionPool db_pool(":memory:");

        // create table with values
        auto conn = db_pool.getConnection();
        conn.exec("CREATE TABLE t1(x INT)");
        conn.exec("INSERT INTO t1(x) VALUES(1)");
        conn.exec("INSERT INTO t1(x) VALUES(5)");

        stmt = conn.getStmt("SELECT * FROM t1 ORDER BY x");
    }

    // make sure that conn falling out of scope doesn't cause the prepared statement to crash
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::row, stmt.execute());
    EXPECT_EQ(stmt.getInt32(0), 1);
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::row, stmt.execute());
    EXPECT_EQ(stmt.getInt32(0), 5);
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::done, stmt.execute());
}

TEST(SQLite3Test, connection_count_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");
    auto const count = db_pool.count();

    auto conn = db_pool.getConnection();
    EXPECT_EQ(count - 1, db_pool.count());

    {
        auto another_conn = db_pool.getConnection();
        EXPECT_EQ(count - 2, db_pool.count());
    }
    EXPECT_EQ(count - 1, db_pool.count());
}