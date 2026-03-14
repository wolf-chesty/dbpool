// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include "dbpool/PreparedStmt.hpp"
#include "dbpool/sqlite/ConnectionPool.hpp"
#include <uuid/uuid.h>
#include <cstring>

TEST(SQLite3Test, uuid_bind_test)
{
	dbpool::sqlite::ConnectionPool db_pool(":memory:");

	auto conn = db_pool.getConnection();
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::ok, conn.exec("CREATE TABLE t1(id INTEGER, test_val BLOB)"));

	auto insert_stmt = conn.getStmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

	int const id0{0};
	insert_stmt.bindInt32(1, id0);

	uuid_t uuid;
	uuid_generate_random(uuid);
	std::array<std::byte, 16> val;
	std::memcpy(val.data(), uuid, 16);

	insert_stmt.bindUuid(2, val);
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::done, insert_stmt.execute());

	auto query_stmt = conn.getStmt("SELECT test_val FROM t1 WHERE id = ?");

	query_stmt.bindInt32(1, id0);
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::row, query_stmt.execute());
	EXPECT_EQ(val, query_stmt.getUuid(0));
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::done, query_stmt.execute());

	EXPECT_THROW(query_stmt.getUuid(-1), std::runtime_error);
	EXPECT_THROW(query_stmt.getUuid(1), std::runtime_error);
}

TEST(SQLite3Test, uuid_invalid_bind_test)
{
	dbpool::sqlite::ConnectionPool db_pool(":memory:");

	auto conn = db_pool.getConnection();
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::ok, conn.exec("CREATE TABLE t1(id INTEGER, test_val BLOB)"));

	std::array<std::byte, 16> val;

	auto insert_stmt = conn.getStmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");
	EXPECT_THROW(insert_stmt.bindUuid(0, val), std::runtime_error);
	insert_stmt.reset();
	EXPECT_THROW(insert_stmt.bindUuid(3, val), std::runtime_error);
}

TEST(SQLite3Test, uuid_null_bind_test)
{
	dbpool::sqlite::ConnectionPool db_pool(":memory:");

	auto conn = db_pool.getConnection();
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::ok, conn.exec("CREATE TABLE t1(id INTEGER, test_val BLOB)"));

	auto insert_stmt = conn.getStmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

	int const id0{0};
	insert_stmt.bindInt32(1, id0);
	insert_stmt.bindNull(2);
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::done, insert_stmt.execute());

	auto query_stmt = conn.getStmt("SELECT test_val FROM t1 WHERE id = ?");

	query_stmt.bindInt32(1, id0);
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::row, query_stmt.execute());
	EXPECT_TRUE(query_stmt.isNull(0));
	EXPECT_THROW(query_stmt.getUuid(0), std::runtime_error);
}