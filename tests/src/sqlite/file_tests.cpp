// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include <boost/test/unit_test.hpp>

#include "dbpool/sqlite/ConnectionPool.hpp"

BOOST_AUTO_TEST_SUITE(dbpool)

BOOST_AUTO_TEST_SUITE(sqlite)

BOOST_AUTO_TEST_SUITE(file_tests)

BOOST_AUTO_TEST_CASE(bad_file)
{
    auto create_conn_pool = []() { dbpool::sqlite::ConnectionPool conn_pool("/dev/null/test.sqlite3"); };
    BOOST_CHECK_THROW(create_conn_pool(), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(file_type)
{
    auto db_pool = std::make_shared<dbpool::sqlite::ConnectionPool>(":memory:");
    auto db_file = std::dynamic_pointer_cast<dbpool::DatabaseFile>(db_pool);
    BOOST_TEST(db_file != nullptr);
    BOOST_TEST(db_file->is_open());
}

BOOST_AUTO_TEST_CASE(file_name)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");
    BOOST_TEST(db_pool.get_filename() == ":memory:");
    BOOST_TEST(db_pool.is_open());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
