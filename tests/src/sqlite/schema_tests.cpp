// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include "dbpool/sqlite/ConnectionPool.hpp"

TEST(SQLite3Test, default_schema)
{
	dbpool::sqlite::ConnectionPool db_pool(":memory:");
	EXPECT_EQ(db_pool.get_schema(), 0);
}

TEST(SQLite3Test, set_schema)
{
	dbpool::sqlite::ConnectionPool db_pool(":memory:");

	auto conn = db_pool.get_conn();
	conn.exec("CREATE TABLE t1(x INT)");

	constexpr int64_t schema{10};
	db_pool.set_schema(schema);
	EXPECT_EQ(schema, db_pool.get_schema());
}
