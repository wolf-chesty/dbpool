// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include <boost/test/unit_test.hpp>

#include "dbpool/PreparedStmt.hpp"
#include "dbpool/sqlite/ConnectionPool.hpp"

BOOST_AUTO_TEST_SUITE(dbpool)

BOOST_AUTO_TEST_SUITE(sqlite)

BOOST_AUTO_TEST_SUITE(scope_tests)

BOOST_AUTO_TEST_CASE(connection_guard_scope_test)
{
    dbpool::Connection conn;
    {
        dbpool::sqlite::ConnectionPool db_pool(":memory:");
        conn = db_pool.get_conn();
    }

    // make sure that conn is not invalid once the connection pool shared pointer has died
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(x INT)")));
}

BOOST_AUTO_TEST_CASE(statement_scope_test)
{
    PreparedStmt stmt;
    {
        dbpool::sqlite::ConnectionPool db_pool(":memory:");

        // create table with values
        auto conn = db_pool.get_conn();
        conn.exec("CREATE TABLE t1(x INT)");
        conn.exec("INSERT INTO t1(x) VALUES(1)");
        conn.exec("INSERT INTO t1(x) VALUES(5)");

        stmt = conn.get_stmt("SELECT * FROM t1 ORDER BY x");
    }

    // make sure that conn falling out of scope doesn't cause the prepared statement to crash
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == stmt.execute()));
    BOOST_TEST((stmt.get_int32(0) == 1));
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == stmt.execute()));
    BOOST_TEST((stmt.get_int32(0) == 5));
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == stmt.execute()));
}

BOOST_AUTO_TEST_CASE(connection_count_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");
    auto const count = db_pool.count();

    auto conn = db_pool.get_conn();
    BOOST_TEST((count - 1 == db_pool.count()));

    {
        auto another_conn = db_pool.get_conn();
        BOOST_TEST((count - 2 == db_pool.count()));
    }
    BOOST_TEST((count - 1 == db_pool.count()));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
