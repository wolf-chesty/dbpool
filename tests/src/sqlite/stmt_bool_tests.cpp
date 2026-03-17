// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include "dbpool/PreparedStmt.hpp"
#include "dbpool/sqlite/ConnectionPool.hpp"

TEST(SQLite3Test, bool_bind_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.getConnection();
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::ok, conn.exec("CREATE TABLE t1(id INTEGER, test_val BOOLEAN)"));

    auto insert_stmt = conn.getStmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};
    insert_stmt.bindInt32(1, id0);
    insert_stmt.bindBool(2, true);
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::done, insert_stmt.execute());

    int const id1{1};
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::ok, insert_stmt.reset());
    insert_stmt.bindInt32(1, id1);
    insert_stmt.bindBool(2, false);
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::done, insert_stmt.execute());

    auto query_stmt = conn.getStmt("SELECT test_val FROM t1 WHERE id = ?");

    query_stmt.bindInt32(1, id0);
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::row, query_stmt.execute());
    EXPECT_TRUE(query_stmt.getBool(0));
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::done, query_stmt.execute());

    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::ok, query_stmt.reset());
    query_stmt.bindInt32(1, id1);
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::row, query_stmt.execute());
    EXPECT_FALSE(query_stmt.getBool(0));
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::done, query_stmt.execute());

    EXPECT_THROW(query_stmt.getBool(-1), std::runtime_error);
    EXPECT_THROW(query_stmt.getBool(1), std::runtime_error);
}

TEST(SQLite3Test, bool_invalid_bind_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.getConnection();
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::ok, conn.exec("CREATE TABLE t1(id INTEGER, test_val BOOLEAN)"));

    auto insert_stmt = conn.getStmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");
    EXPECT_THROW(insert_stmt.bindBool(0, true), std::runtime_error);
    insert_stmt.reset();
    EXPECT_THROW(insert_stmt.bindBool(3, true), std::runtime_error);
}

TEST(SQLite3Test, bool_null_bind_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.getConnection();
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::ok, conn.exec("CREATE TABLE t1(id INTEGER, test_val BOOLEAN)"));

    auto insert_stmt = conn.getStmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};
    insert_stmt.bindInt32(1, id0);
    insert_stmt.bindNull(2);
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::done, insert_stmt.execute());

    auto query_stmt = conn.getStmt("SELECT test_val FROM t1 WHERE id = ?");
    query_stmt.bindInt32(1, id0);
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::row, query_stmt.execute());
    EXPECT_TRUE(query_stmt.isNull(0));
    EXPECT_THROW(query_stmt.getBool(0), std::runtime_error);
}