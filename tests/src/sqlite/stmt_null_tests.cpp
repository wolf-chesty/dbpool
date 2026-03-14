// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include "dbpool/PreparedStmt.hpp"
#include "dbpool/sqlite/ConnectionPool.hpp"

TEST(SQLite3Test, null_bind_test)
{
	dbpool::sqlite::ConnectionPool db_pool(":memory:");

	auto conn = db_pool.getConnection();
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::ok, conn.exec("CREATE TABLE t1(id INTEGER, test_val INTEGER)"));

	auto insert_stmt = conn.getStmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");
	EXPECT_THROW(insert_stmt.bindNull(0), std::runtime_error);
	EXPECT_THROW(insert_stmt.bindNull(3), std::runtime_error);
}