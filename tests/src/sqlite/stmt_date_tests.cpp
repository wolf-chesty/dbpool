// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include "dbpool/PreparedStmt.hpp"
#include "dbpool/sqlite/ConnectionPool.hpp"

TEST(SQLite3Test, date_bind_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.getConnection();
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::ok, conn.exec("CREATE TABLE t1(id INTEGER, test_val INTEGER)"));

    int const id0{0};
    std::string_view date{"2026-02-18T14:30:05.000Z"};

    auto insert_stmt = conn.getStmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");
    insert_stmt.bindInt32(1, id0);
    insert_stmt.bindDate(2, date);
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::done, insert_stmt.execute());

    auto query_stmt = conn.getStmt("SELECT test_val FROM t1 WHERE id = ?");
    query_stmt.bindInt32(1, id0);
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::row, query_stmt.execute());
    EXPECT_EQ(date, query_stmt.getDate(0));

    EXPECT_THROW(query_stmt.getDate(-1), std::runtime_error);
    EXPECT_THROW(query_stmt.getDate(1), std::runtime_error);
}

TEST(SQLite3Test, date_invalid_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.getConnection();
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::ok, conn.exec("CREATE TABLE t1(id INTEGER, test_val INTEGER)"));

    int const id0{0};
    std::string_view date{"meh"};

    auto insert_stmt = conn.getStmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");
    insert_stmt.bindInt32(1, id0);
    EXPECT_THROW(insert_stmt.bindDate(2, date), std::runtime_error);
}

TEST(SQLite3Test, date_emtpy_bind_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.getConnection();
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::ok, conn.exec("CREATE TABLE t1(id INTEGER, test_val INTEGER)"));

    int const id0{0};
    std::string_view date;

    auto insert_stmt = conn.getStmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");
    insert_stmt.bindInt32(1, id0);
    insert_stmt.bindDate(2, date);
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::done, insert_stmt.execute());

    auto query_stmt = conn.getStmt("SELECT test_val FROM t1 WHERE id = ?");
    query_stmt.bindInt32(1, id0);
    EXPECT_THROW(query_stmt.getDate(0), std::runtime_error);
}

TEST(SQLite3Test, date_null_bind_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.getConnection();
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::ok, conn.exec("CREATE TABLE t1(id INTEGER, test_val INTEGER)"));

    auto insert_stmt = conn.getStmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};
    insert_stmt.bindInt32(1, id0);
    insert_stmt.bindNull(2);
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::done, insert_stmt.execute());
    auto query_stmt = conn.getStmt("SELECT test_val FROM t1 WHERE id = ?");
    query_stmt.bindInt32(1, id0);
    EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::row, query_stmt.execute());
    EXPECT_TRUE(query_stmt.isNull(0));
    EXPECT_THROW(query_stmt.getDate(0), std::runtime_error);
}