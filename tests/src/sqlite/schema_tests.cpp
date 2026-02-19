// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include <boost/test/unit_test.hpp>

#include "dbpool/sqlite/ConnectionPool.hpp"

BOOST_AUTO_TEST_SUITE(dbpool)

BOOST_AUTO_TEST_SUITE(sqlite)

BOOST_AUTO_TEST_SUITE(schema_tests)

BOOST_AUTO_TEST_CASE(default_schema)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");
    BOOST_TEST(db_pool.get_schema() == 0);
}

BOOST_AUTO_TEST_CASE(set_schema)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    conn.exec("CREATE TABLE t1(x INT)");

    constexpr int64_t schema{10};
    db_pool.set_schema(schema);
    BOOST_TEST(schema == db_pool.get_schema());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
