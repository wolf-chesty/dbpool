// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include "dbpool/PreparedStmt.hpp"
#include "dbpool/sqlite/ConnectionPool.hpp"
#include <cstdlib>

TEST(SQLite3Test, blob_bind_test)
{
	std::srand(std::time({}));

	dbpool::sqlite::ConnectionPool db_pool(":memory:");

	auto conn = db_pool.getConnection();
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::ok, conn.exec("CREATE TABLE t1(id INTEGER, test_val BLOB)"));

	auto insert_stmt = conn.getStmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

	int const id0{0};

	std::array<std::byte, 256> blob;
	for (auto val : blob) {
		val = static_cast<std::byte>(std::rand());
	}

	insert_stmt.bindInt32(1, id0);
	insert_stmt.bindBlob(2, blob);
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::done, insert_stmt.execute());

	auto query_stmt = conn.getStmt("SELECT test_val FROM t1 WHERE id = ?");
	query_stmt.bindInt32(1, id0);
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::row, query_stmt.execute());
	auto result = query_stmt.getBlob(0);
	EXPECT_TRUE(std::equal(blob.begin(), blob.end(), result.begin(), result.end()));

	EXPECT_THROW(query_stmt.getBlob(-1), std::runtime_error);
	EXPECT_THROW(query_stmt.getBlob(1), std::runtime_error);
}

TEST(SQLite3Test, blob_invalid_bind_test)
{
	dbpool::sqlite::ConnectionPool db_pool(":memory:");

	auto conn = db_pool.getConnection();
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::ok, conn.exec("CREATE TABLE t1(id INTEGER, test_val BLOB)"));

	std::array<std::byte, 256> blob;

	auto insert_stmt = conn.getStmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");
	EXPECT_THROW(insert_stmt.bindBlob(0, blob), std::runtime_error);
	insert_stmt.reset();
	EXPECT_THROW(insert_stmt.bindBlob(3, blob), std::runtime_error);
}

TEST(SQLite3Test, blob_null_bind_test)
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
	EXPECT_THROW(query_stmt.getBlob(0), std::runtime_error);
}