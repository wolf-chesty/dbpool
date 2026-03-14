// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include "dbpool/sqlite/ConnectionPool.hpp"

TEST(SQLite3Test, bad_file)
{
	auto create_conn_pool = []() { dbpool::sqlite::ConnectionPool conn_pool("/dev/null/test.sqlite3"); };
	EXPECT_THROW(create_conn_pool(), std::runtime_error);
}

TEST(SQLite3Test, file_type)
{
	auto db_pool = std::make_shared<dbpool::sqlite::ConnectionPool>(":memory:");
	auto db_file = std::dynamic_pointer_cast<dbpool::DatabaseFile>(db_pool);
	EXPECT_NE(db_file, nullptr);
	EXPECT_TRUE(db_file->isOpen());
}

TEST(SQLite3Test, file_name)
{
	dbpool::sqlite::ConnectionPool db_pool(":memory:");
	EXPECT_EQ(db_pool.getFilename(), ":memory:");
	EXPECT_TRUE(db_pool.isOpen());
}