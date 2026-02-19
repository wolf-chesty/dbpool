// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include <boost/test/unit_test.hpp>

#include "dbpool/PreparedStmt.hpp"
#include "dbpool/sqlite/ConnectionPool.hpp"

BOOST_AUTO_TEST_SUITE(dbpool)

BOOST_AUTO_TEST_SUITE(sqlite)

BOOST_AUTO_TEST_SUITE(stmt_double_tests)

BOOST_AUTO_TEST_CASE(bind_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val REAL)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};
    insert_stmt.bind_int32(1, id0);
    insert_stmt.bind_double(2, 3.14159);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    auto query_stmt = conn.get_stmt("SELECT test_val FROM t1 WHERE id = ?");

    query_stmt.bind_int32(1, id0);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == query_stmt.execute()));
    BOOST_TEST((3.14159 == query_stmt.get_double(0)));
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == query_stmt.execute()));

    BOOST_CHECK_THROW(query_stmt.get_double(-1), std::runtime_error);
    BOOST_CHECK_THROW(query_stmt.get_double(1), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(invalid_bind_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val REAL)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");
    BOOST_CHECK_THROW(insert_stmt.bind_double(0, 1.0), std::runtime_error);
    insert_stmt.reset();
    BOOST_CHECK_THROW(insert_stmt.bind_double(3, 1.0), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(null_bind_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val REAL)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};
    insert_stmt.bind_int32(1, id0);
    insert_stmt.bind_null(2);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    auto query_stmt = conn.get_stmt("SELECT test_val FROM t1 WHERE id = ?");

    query_stmt.bind_int32(1, id0);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == query_stmt.execute()));
    BOOST_TEST(query_stmt.is_null(0));
    BOOST_CHECK_THROW(query_stmt.get_double(0), std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
