// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include <gtest/gtest.h>

#include "dbpool/PreparedStmt.hpp"
#include "dbpool/sqlite/ConnectionPool.hpp"
#include <cstdlib>

TEST(SQLite3Test, prep_stmt_test)
{
	dbpool::sqlite::ConnectionPool db_pool(":memory:");
	db_pool.setPrepSql("CREATE TABLE IF NOT EXISTS t1 (x INT)");

	// Checking out a connection out will create table t1
	auto conn = db_pool.getConnection();

	// Check if the table exists
	auto stmt = conn.getStmt("SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name='t1'");
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::row, stmt.execute());
	EXPECT_EQ(1, stmt.getInt32(0));
}

TEST(SQLite3Test, table_creation)
{
	dbpool::sqlite::ConnectionPool db_pool(":memory:");

	auto conn = db_pool.getConnection();
	auto ret = conn.exec("CREATE TABLE t1(x INT)");
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::ok, ret);

	auto stmt = conn.getStmt("SELECT name FROM sqlite_master WHERE type='table' and name='t1'");
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::row, stmt.execute());

	db_pool.commit();

	auto const name = stmt.getText(0);
	EXPECT_EQ(name, "t1");

	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::done, stmt.execute());
}

TEST(SQLite3Test, table_insertion)
{
	dbpool::sqlite::ConnectionPool db_pool(":memory:");

	auto conn = db_pool.getConnection();
	conn.exec("CREATE TABLE t1(x INT)");

	auto stmt = conn.getStmt("INSERT INTO t1(x) VALUES(?)");

	constexpr int32_t val{10};
	stmt.bindInt32(1, val);
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::done, stmt.execute());
}

TEST(SQLite3Test, table_deletion)
{
	dbpool::sqlite::ConnectionPool db_pool(":memory:");

	auto conn = db_pool.getConnection();
	conn.exec("CREATE TABLE t1(x INT)");

	auto stmt = conn.getStmt("INSERT INTO t1(x) VALUES(?)");
	constexpr int32_t val{10};
	stmt.bindInt32(1, val);
	stmt.execute();

	stmt = conn.getStmt("DELETE FROM t1 WHERE x = ?");
	stmt.bindInt32(1, val);
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::done, stmt.execute());

	stmt = conn.getStmt("SELECT x from t1");
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::done, stmt.execute());
}

TEST(SQLite3Test, table_selection)
{
	dbpool::sqlite::ConnectionPool db_pool(":memory:");

	auto conn = db_pool.getConnection();
	conn.exec("CREATE TABLE t1(x INT)");

	auto stmt = conn.getStmt("INSERT INTO t1(x) VALUES(?)");

	constexpr int32_t val{10};
	stmt.bindInt32(1, val);
	stmt.execute();

	stmt = conn.getStmt("SELECT x FROM t1");

	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::row, stmt.execute());
	EXPECT_EQ(stmt.getInt64(0), val);

	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::done, stmt.execute());
}

TEST(SQLite3Test, thread_test)
{
	dbpool::sqlite::ConnectionPool db_pool(":memory:");

	// create table with values
	auto conn1 = db_pool.getConnection();
	conn1.exec("CREATE TABLE t1(x INT)");
	conn1.exec("INSERT INTO t1(x) VALUES(1)");
	conn1.exec("INSERT INTO t1(x) VALUES(5)");

	// create another table with single value
	auto conn2 = db_pool.getConnection();
	conn2.exec("CREATE TABLE t2(x INT)");
	conn2.exec("INSERT INTO t2(x) VALUES(1)");

	auto stmt1 = conn1.getStmt("SELECT * FROM t1 ORDER BY x");
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::row, stmt1.execute());

	auto stmt2 = conn2.getStmt("SELECT * FROM t2 ORDER BY x");
	auto thread2 = [&stmt2]() { EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::row, stmt2.execute()); };

	// execute stmt2 while stmt1 still has a selection index and wait for stmt2 to complete; both selection indices
	// should be correct
	std::thread t(thread2);
	t.join();

	// make sure t1 index is still valid
	EXPECT_EQ(stmt1.getInt32(0), 1);
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::row, stmt1.execute());
	EXPECT_EQ(stmt1.getInt32(0), 5);
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::done, stmt1.execute());

	// make sure t2 index is still valid
	EXPECT_EQ(stmt2.getInt32(0), 1);
	EXPECT_EQ(dbpool::PreparedStmt::ReturnCode::done, stmt2.execute());
}

TEST(SQLite3Test, isNull_tests)
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

	EXPECT_THROW(query_stmt.isNull(-1), std::runtime_error);
	EXPECT_THROW(query_stmt.isNull(1), std::runtime_error);
}