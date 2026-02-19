// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include <boost/test/unit_test.hpp>

#include "dbpool/PreparedStmt.hpp"
#include "dbpool/sqlite/ConnectionPool.hpp"
#include <chrono>
#include <cstdlib>
#include <uuid/uuid.h>

BOOST_AUTO_TEST_SUITE(dbpool)

BOOST_AUTO_TEST_SUITE(sqlite)

BOOST_AUTO_TEST_CASE(bad_file)
{
    BOOST_CHECK_THROW(std::make_shared<dbpool::sqlite::ConnectionPool>("/dev/null/test.sqlite3"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(file_type)
{
    auto db_pool = std::make_shared<dbpool::sqlite::ConnectionPool>(":memory:");
    auto db_file = std::dynamic_pointer_cast<dbpool::DatabaseFile>(db_pool);
    BOOST_TEST(db_file != nullptr);
}

BOOST_AUTO_TEST_CASE(file_name)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");
    BOOST_TEST(db_pool.get_filename() == ":memory:");
    BOOST_TEST(db_pool.is_open());
}

BOOST_AUTO_TEST_CASE(schema)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");
    db_pool.set_schema(100100);
    BOOST_TEST((100100 == db_pool.get_schema()));
}

BOOST_AUTO_TEST_CASE(table_creation)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    auto ret = conn.exec("CREATE TABLE t1(x INT)");
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == ret));

    auto stmt = conn.get_stmt("SELECT name FROM sqlite_master WHERE type='table' and name='t1'");
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == stmt.execute()));

    db_pool.commit();

    auto const name = stmt.get_text(0);
    BOOST_TEST(name == "t1");

    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == stmt.execute()));
}

BOOST_AUTO_TEST_CASE(table_insertion)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    conn.exec("CREATE TABLE t1(x INT)");

    auto stmt = conn.get_stmt("INSERT INTO t1(x) VALUES(?)");

    constexpr int32_t val{10};
    stmt.bind_int32(1, val);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == stmt.execute()));
}

BOOST_AUTO_TEST_CASE(table_deletion)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    conn.exec("CREATE TABLE t1(x INT)");

    auto stmt = conn.get_stmt("INSERT INTO t1(x) VALUES(?)");
    constexpr int32_t val{10};
    stmt.bind_int32(1, val);
    stmt.execute();

    stmt = conn.get_stmt("DELETE FROM t1 WHERE x = ?");
    stmt.bind_int32(1, val);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == stmt.execute()));

    stmt = conn.get_stmt("SELECT x from t1");
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == stmt.execute()));
}

BOOST_AUTO_TEST_CASE(table_selection)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    conn.exec("CREATE TABLE t1(x INT)");

    auto stmt = conn.get_stmt("INSERT INTO t1(x) VALUES(?)");

    constexpr int32_t val{10};
    stmt.bind_int32(1, val);
    stmt.execute();

    stmt = conn.get_stmt("SELECT x FROM t1");

    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == stmt.execute()));
    BOOST_TEST(stmt.get_int64(0) == val);

    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == stmt.execute()));
}

BOOST_AUTO_TEST_CASE(bad_bind_index)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    conn.exec("CREATE TABLE t1(x INT)");

    auto stmt = conn.get_stmt("INSERT INTO t1(x) VALUES(?)");
    BOOST_CHECK_THROW(stmt.bind_int32(0, 0), std::runtime_error);
    BOOST_CHECK_THROW(stmt.bind_int32(2, 0), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(bad_get_index)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    conn.exec("CREATE TABLE t1(x INT)");

    auto stmt = conn.get_stmt("INSERT INTO t1(x) VALUES(?)");
    stmt.bind_int32(1, 10);
    stmt.execute();

    stmt = conn.get_stmt("SELECT x FROM t1");
    stmt.execute();

    BOOST_TEST((stmt.get_int64(0) == 10));
    BOOST_CHECK_THROW(stmt.get_int64(1), std::runtime_error);
}

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

BOOST_AUTO_TEST_CASE(thread_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    // create table with values
    auto conn1 = db_pool.get_conn();
    conn1.exec("CREATE TABLE t1(x INT)");
    conn1.exec("INSERT INTO t1(x) VALUES(1)");
    conn1.exec("INSERT INTO t1(x) VALUES(5)");

    // create another table with single value
    auto conn2 = db_pool.get_conn();
    conn2.exec("CREATE TABLE t2(x INT)");
    conn2.exec("INSERT INTO t2(x) VALUES(1)");

    auto stmt1 = conn1.get_stmt("SELECT * FROM t1 ORDER BY x");
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == stmt1.execute()));

    auto stmt2 = conn2.get_stmt("SELECT * FROM t2 ORDER BY x");
    auto thread2 = [&stmt2]() { BOOST_TEST((PreparedStmt::ReturnCode::row == stmt2.execute())); };

    // execute stmt2 while stmt1 still has a selection index and wait for stmt2 to complete; both selection indices
    // should be correct
    std::thread t(thread2);
    t.join();

    // make sure t1 index is still valid
    BOOST_TEST((stmt1.get_int32(0) == 1));
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == stmt1.execute()));
    BOOST_TEST((stmt1.get_int32(0) == 5));
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == stmt1.execute()));

    // make sure t2 index is still valid
    BOOST_TEST((stmt2.get_int32(0) == 1));
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == stmt2.execute()));
}

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

BOOST_AUTO_TEST_CASE(stmt_blob_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val BLOB)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};

    std::array<std::byte, 256> blob;
    for (auto val : blob) {
        val = static_cast<std::byte>(std::rand());
    }

    insert_stmt.bind_int32(1, id0);
    insert_stmt.bind_blob(2, blob);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    auto query_stmt = conn.get_stmt("SELECT test_val FROM t1 WHERE id = ?");
    query_stmt.bind_int32(1, id0);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == query_stmt.execute()));
    auto result = query_stmt.get_blob(0);
    BOOST_TEST((std::equal(blob.begin(), blob.end(), result.begin(), result.end())));

    BOOST_CHECK_THROW(query_stmt.get_blob(-1), std::runtime_error);
    BOOST_CHECK_THROW(query_stmt.get_blob(1), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_bind_blob_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val BLOB)")));

    std::array<std::byte, 256> blob;

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");
    BOOST_CHECK_THROW(insert_stmt.bind_blob(0, blob), std::runtime_error);
    insert_stmt.reset();
    BOOST_CHECK_THROW(insert_stmt.bind_blob(3, blob), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_empty_blob_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val BLOB)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};
    std::vector<std::byte> blob;
    insert_stmt.bind_int32(1, id0);
    insert_stmt.bind_blob(2, blob);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    auto query_stmt = conn.get_stmt("SELECT test_val FROM t1 WHERE id = ?");
    query_stmt.bind_int32(1, id0);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == query_stmt.execute()));
    BOOST_TEST(query_stmt.is_null(0));
    BOOST_CHECK_THROW(query_stmt.get_blob(0), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_null_blob_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val BLOB)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};
    insert_stmt.bind_int32(1, id0);
    insert_stmt.bind_null(2);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    auto query_stmt = conn.get_stmt("SELECT test_val FROM t1 WHERE id = ?");
    query_stmt.bind_int32(1, id0);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == query_stmt.execute()));
    BOOST_TEST(query_stmt.is_null(0));
    BOOST_CHECK_THROW(query_stmt.get_blob(0), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_bool_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val BOOLEAN)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};
    insert_stmt.bind_int32(1, id0);
    insert_stmt.bind_bool(2, true);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    int const id1{1};
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == insert_stmt.reset()));
    insert_stmt.bind_int32(1, id1);
    insert_stmt.bind_bool(2, false);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    auto query_stmt = conn.get_stmt("SELECT test_val FROM t1 WHERE id = ?");

    query_stmt.bind_int32(1, id0);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == query_stmt.execute()));
    BOOST_TEST((true == query_stmt.get_bool(0)));
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == query_stmt.execute()));

    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == query_stmt.reset()));
    query_stmt.bind_int32(1, id1);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == query_stmt.execute()));
    BOOST_TEST((false == query_stmt.get_bool(0)));
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == query_stmt.execute()));

    BOOST_CHECK_THROW(query_stmt.get_bool(-1), std::runtime_error);
    BOOST_CHECK_THROW(query_stmt.get_bool(1), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_bind_bool_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val BOOLEAN)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");
    BOOST_CHECK_THROW(insert_stmt.bind_bool(0, true), std::runtime_error);
    insert_stmt.reset();
    BOOST_CHECK_THROW(insert_stmt.bind_bool(3, true), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_null_bool_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val BOOLEAN)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};
    insert_stmt.bind_int32(1, id0);
    insert_stmt.bind_null(2);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    auto query_stmt = conn.get_stmt("SELECT test_val FROM t1 WHERE id = ?");
    query_stmt.bind_int32(1, id0);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == query_stmt.execute()));
    BOOST_TEST(query_stmt.is_null(0));
    BOOST_CHECK_THROW(query_stmt.get_bool(0), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_date_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val INTEGER)")));

    int const id0{0};
    std::string_view date{"2026-02-18T14:30:05.000Z"};

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");
    insert_stmt.bind_int32(1, id0);
    insert_stmt.bind_date(2, date);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    auto query_stmt = conn.get_stmt("SELECT test_val FROM t1 WHERE id = ?");
    query_stmt.bind_int32(1, id0);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == query_stmt.execute()));
    BOOST_TEST((date == query_stmt.get_date(0)));

    BOOST_CHECK_THROW(query_stmt.get_date(-1), std::runtime_error);
    BOOST_CHECK_THROW(query_stmt.get_date(1), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_bad_date_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val INTEGER)")));

    int const id0{0};
    std::string_view date{"meh"};

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");
    insert_stmt.bind_int32(1, id0);
    BOOST_CHECK_THROW(insert_stmt.bind_date(2, date), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_emtpy_date_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val INTEGER)")));

    int const id0{0};
    std::string_view date;

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");
    insert_stmt.bind_int32(1, id0);
    insert_stmt.bind_date(2, date);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    auto query_stmt = conn.get_stmt("SELECT test_val FROM t1 WHERE id = ?");
    query_stmt.bind_int32(1, id0);
    BOOST_CHECK_THROW(query_stmt.get_date(0), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_double_test)
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

BOOST_AUTO_TEST_CASE(stmt_bind_double_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val REAL)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");
    BOOST_CHECK_THROW(insert_stmt.bind_double(0, 1.0), std::runtime_error);
    insert_stmt.reset();
    BOOST_CHECK_THROW(insert_stmt.bind_double(3, 1.0), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_null_double_test)
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

BOOST_AUTO_TEST_CASE(stmt_int32_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val INTEGER)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};
    int const val{32};
    insert_stmt.bind_int32(1, id0);
    insert_stmt.bind_int32(2, val);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    auto query_stmt = conn.get_stmt("SELECT test_val FROM t1 WHERE id = ?");

    query_stmt.bind_int32(1, id0);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == query_stmt.execute()));
    BOOST_TEST((val == query_stmt.get_int32(0)));
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == query_stmt.execute()));

    BOOST_CHECK_THROW(query_stmt.get_int32(-1), std::runtime_error);
    BOOST_CHECK_THROW(query_stmt.get_int32(1), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_bind_int32_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val INTEGER)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");
    BOOST_CHECK_THROW(insert_stmt.bind_int32(0, 0), std::runtime_error);
    insert_stmt.reset();
    BOOST_CHECK_THROW(insert_stmt.bind_int32(3, 0), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_null_int32_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val INTEGER)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};
    insert_stmt.bind_int32(1, id0);
    insert_stmt.bind_null(2);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    auto query_stmt = conn.get_stmt("SELECT test_val FROM t1 WHERE id = ?");

    query_stmt.bind_int32(1, id0);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == query_stmt.execute()));
    BOOST_TEST(query_stmt.is_null(0));
    BOOST_CHECK_THROW(query_stmt.get_int32(0), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_int64_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val INTEGER)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};
    int64_t const val{32};
    insert_stmt.bind_int32(1, id0);
    insert_stmt.bind_int64(2, val);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    auto query_stmt = conn.get_stmt("SELECT test_val FROM t1 WHERE id = ?");

    query_stmt.bind_int32(1, id0);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == query_stmt.execute()));
    BOOST_TEST((val == query_stmt.get_int64(0)));
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == query_stmt.execute()));

    BOOST_CHECK_THROW(query_stmt.get_int64(-1), std::runtime_error);
    BOOST_CHECK_THROW(query_stmt.get_int64(1), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_bind_int64_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val INTEGER)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");
    BOOST_CHECK_THROW(insert_stmt.bind_int64(0, 0), std::runtime_error);
    insert_stmt.reset();
    BOOST_CHECK_THROW(insert_stmt.bind_int64(3, 0), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_null_int64_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val INTEGER)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};
    insert_stmt.bind_int32(1, id0);
    insert_stmt.bind_null(2);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    auto query_stmt = conn.get_stmt("SELECT test_val FROM t1 WHERE id = ?");

    query_stmt.bind_int32(1, id0);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == query_stmt.execute()));
    BOOST_TEST(query_stmt.is_null(0));
    BOOST_CHECK_THROW(query_stmt.get_int64(0), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_text_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val TEXT)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};
    std::string const val{"This is a test"};
    insert_stmt.bind_int32(1, id0);
    insert_stmt.bind_text(2, val);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    auto query_stmt = conn.get_stmt("SELECT test_val FROM t1 WHERE id = ?");

    query_stmt.bind_int32(1, id0);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == query_stmt.execute()));
    BOOST_TEST((val == query_stmt.get_text(0)));
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == query_stmt.execute()));

    BOOST_CHECK_THROW(query_stmt.get_text(-1), std::runtime_error);
    BOOST_CHECK_THROW(query_stmt.get_text(1), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_bind_text_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val TEXT)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");
    BOOST_CHECK_THROW(insert_stmt.bind_text(0, "Test"), std::runtime_error);
    insert_stmt.reset();
    BOOST_CHECK_THROW(insert_stmt.bind_text(3, "Test"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_empty_text_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val TEXT)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};
    std::string const val;
    insert_stmt.bind_int32(1, id0);
    insert_stmt.bind_null(2);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    auto query_stmt = conn.get_stmt("SELECT test_val FROM t1 WHERE id = ?");

    query_stmt.bind_int32(1, id0);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == query_stmt.execute()));
    BOOST_TEST(query_stmt.is_null(0));
    BOOST_CHECK_THROW(query_stmt.get_text(0), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_null_text_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val TEXT)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};
    std::string const val{"This is a test"};
    insert_stmt.bind_int32(1, id0);
    insert_stmt.bind_null(2);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    auto query_stmt = conn.get_stmt("SELECT test_val FROM t1 WHERE id = ?");

    query_stmt.bind_int32(1, id0);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == query_stmt.execute()));
    BOOST_TEST(query_stmt.is_null(0));
    BOOST_CHECK_THROW(query_stmt.get_text(0), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_text16_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val TEXT)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};
    std::u16string const val{u"Hello, 世界"};
    insert_stmt.bind_int32(1, id0);
    insert_stmt.bind_text16(2, val);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    auto query_stmt = conn.get_stmt("SELECT test_val FROM t1 WHERE id = ?");

    query_stmt.bind_int32(1, id0);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == query_stmt.execute()));
    BOOST_TEST((val == query_stmt.get_text16(0)));
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == query_stmt.execute()));

    BOOST_CHECK_THROW(query_stmt.get_text16(-1), std::runtime_error);
    BOOST_CHECK_THROW(query_stmt.get_text16(1), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_bind_text16_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val TEXT)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");
    BOOST_CHECK_THROW(insert_stmt.bind_text16(0, u"Test"), std::runtime_error);
    insert_stmt.reset();
    BOOST_CHECK_THROW(insert_stmt.bind_text16(3, u"Test"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_empty_text16_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val TEXT)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};
    std::u16string const val;
    insert_stmt.bind_int32(1, id0);
    insert_stmt.bind_null(2);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    auto query_stmt = conn.get_stmt("SELECT test_val FROM t1 WHERE id = ?");

    query_stmt.bind_int32(1, id0);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == query_stmt.execute()));
    BOOST_TEST(query_stmt.is_null(0));
    BOOST_CHECK_THROW(query_stmt.get_text16(0), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_null_text16_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val TEXT)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};
    std::u16string const val{u"Hello, 世界"};
    insert_stmt.bind_int32(1, id0);
    insert_stmt.bind_null(2);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    auto query_stmt = conn.get_stmt("SELECT test_val FROM t1 WHERE id = ?");

    query_stmt.bind_int32(1, id0);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == query_stmt.execute()));
    BOOST_TEST(query_stmt.is_null(0));
    BOOST_CHECK_THROW(query_stmt.get_text16(0), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_uuid_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val BLOB)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};
    insert_stmt.bind_int32(1, id0);

    uuid_t uuid;
    uuid_generate_random(uuid);
    std::array<std::byte, 16> val;
    std::memcpy(val.data(), uuid, 16);

    insert_stmt.bind_uuid(2, val);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    auto query_stmt = conn.get_stmt("SELECT test_val FROM t1 WHERE id = ?");

    query_stmt.bind_int32(1, id0);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == query_stmt.execute()));
    BOOST_TEST((val == query_stmt.get_uuid(0)));
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == query_stmt.execute()));

    BOOST_CHECK_THROW(query_stmt.get_uuid(-1), std::runtime_error);
    BOOST_CHECK_THROW(query_stmt.get_uuid(1), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_bind_uuid_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val BLOB)")));

    std::array<std::byte, 16> val;

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");
    BOOST_CHECK_THROW(insert_stmt.bind_uuid(0, val), std::runtime_error);
    insert_stmt.reset();
    BOOST_CHECK_THROW(insert_stmt.bind_uuid(3, val), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(stmt_null_uuid_test)
{
    dbpool::sqlite::ConnectionPool db_pool(":memory:");

    auto conn = db_pool.get_conn();
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::ok == conn.exec("CREATE TABLE t1(id INTEGER, test_val BLOB)")));

    auto insert_stmt = conn.get_stmt("INSERT INTO t1 (id, test_val) VALUES (?, ?)");

    int const id0{0};
    insert_stmt.bind_int32(1, id0);
    insert_stmt.bind_null(2);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::done == insert_stmt.execute()));

    auto query_stmt = conn.get_stmt("SELECT test_val FROM t1 WHERE id = ?");

    query_stmt.bind_int32(1, id0);
    BOOST_TEST((dbpool::PreparedStmt::ReturnCode::row == query_stmt.execute()));
    BOOST_TEST(query_stmt.is_null(0));
    BOOST_CHECK_THROW(query_stmt.get_uuid(0), std::runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
