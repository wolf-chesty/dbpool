#include <boost/test/unit_test.hpp>
#include "sqlite/sqlite_conn_pool.h"
#include "db_conn.h"
#include "db_stmt.h"

BOOST_AUTO_TEST_SUITE(dbpool)

BOOST_AUTO_TEST_SUITE(sqlite)

BOOST_AUTO_TEST_CASE(bad_file)
{
	BOOST_CHECK_THROW(sqlite_conn_pool::create("/dev/null/test.sqlite3"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(file_type)
{
	auto dbPool = sqlite_conn_pool::create(":memory:");
	auto dbFile = std::dynamic_pointer_cast<db_file>(dbPool);
	BOOST_TEST(dbFile != nullptr);
}

BOOST_AUTO_TEST_CASE(file_name)
{
	auto dbPool = sqlite_conn_pool::create(":memory:");
	auto dbFile = std::dynamic_pointer_cast<db_file>(dbPool);
	BOOST_TEST(dbFile->get_filename() == ":memory:");
}

BOOST_AUTO_TEST_CASE(table_creation)
{
	auto dbPool = sqlite_conn_pool::create(":memory:");

	auto conn = dbPool->get_conn();
	auto ret = conn->exec("CREATE TABLE t1(x INT)");
	BOOST_TEST((db_stmt::return_code::ok == ret));

	auto stmt = conn->get_stmt("SELECT name FROM sqlite_master WHERE type='table' and name='t1'");
	BOOST_TEST((stmt != nullptr));

	BOOST_TEST((db_stmt::return_code::row == stmt->execute()));

	const auto name = stmt->get_text(0);
	BOOST_TEST(name == "t1");

	BOOST_TEST((db_stmt::return_code::done == stmt->execute()));
}

BOOST_AUTO_TEST_CASE(table_insertion)
{
	auto dbPool = sqlite_conn_pool::create(":memory:");

	auto conn = dbPool->get_conn();
	conn->exec("CREATE TABLE t1(x INT)");

	auto stmt = conn->get_stmt("INSERT INTO t1(x) VALUES(?)");
	BOOST_TEST((stmt != nullptr));

	const int32_t val{10};
	stmt->bind_int32(1, val);
	BOOST_TEST((db_stmt::return_code::done == stmt->execute()));
}

BOOST_AUTO_TEST_CASE(table_deletion)
{
	auto dbPool = sqlite_conn_pool::create(":memory:");

	auto conn = dbPool->get_conn();
	conn->exec("CREATE TABLE t1(x INT)");

	auto stmt = conn->get_stmt("INSERT INTO t1(x) VALUES(?)");
	const int32_t val{10};
	stmt->bind_int32(1, val);
	stmt->execute();

	stmt = conn->get_stmt("DELETE FROM t1 WHERE x = ?");
	stmt->bind_int32(1, val);
	BOOST_TEST((db_stmt::return_code::done == stmt->execute()));

	stmt = conn->get_stmt("SELECT x from t1");
	BOOST_TEST((db_stmt::return_code::done == stmt->execute()));
}

BOOST_AUTO_TEST_CASE(table_selection)
{
	auto dbPool = sqlite_conn_pool::create(":memory:");

	auto conn = dbPool->get_conn();
	conn->exec("CREATE TABLE t1(x INT)");

	auto stmt = conn->get_stmt("INSERT INTO t1(x) VALUES(?)");

	const int32_t val{10};
	stmt->bind_int32(1, val);
	stmt->execute();

	stmt = conn->get_stmt("SELECT x FROM t1");
	BOOST_TEST((stmt != nullptr));

	BOOST_TEST((db_stmt::return_code::row == stmt->execute()));
	BOOST_TEST(stmt->get_int64(0) == val);

	BOOST_TEST((db_stmt::return_code::done == stmt->execute()));
}

BOOST_AUTO_TEST_CASE(bad_bind_index)
{
	auto dbPool = sqlite_conn_pool::create(":memory:");

	auto conn = dbPool->get_conn();
	conn->exec("CREATE TABLE t1(x INT)");

	auto stmt = conn->get_stmt("INSERT INTO t1(x) VALUES(?)");
	BOOST_CHECK_THROW(stmt->bind_int32(0, 0), std::runtime_error);
	BOOST_CHECK_THROW(stmt->bind_int32(2, 0), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(bad_get_index)
{
	auto dbPool = sqlite_conn_pool::create(":memory:");

	auto conn = dbPool->get_conn();
	conn->exec("CREATE TABLE t1(x INT)");

	auto stmt = conn->get_stmt("INSERT INTO t1(x) VALUES(?)");
	stmt->bind_int32(1, 10);
	stmt->execute();

	stmt = conn->get_stmt("SELECT x FROM t1");
	stmt->execute();

	BOOST_TEST((stmt->get_int64(1) == 0));
}

BOOST_AUTO_TEST_CASE(default_schema)
{
	auto dbPool = sqlite_conn_pool::create(":memory:");
	BOOST_TEST(dbPool->get_schema() == 0);
}

BOOST_AUTO_TEST_CASE(set_schema)
{
	auto dbPool = sqlite_conn_pool::create(":memory:");

	auto conn = dbPool->get_conn();
	conn->exec("CREATE TABLE t1(x INT)");

	const int64_t schema{10};
	dbPool->set_schema(schema);
	BOOST_TEST(schema == dbPool->get_schema());
}

BOOST_AUTO_TEST_CASE(thread_test)
{
	auto dbPool = sqlite_conn_pool::create(":memory:");

	// create table with values
	auto conn1 = dbPool->get_conn();
	conn1->exec("CREATE TABLE t1(x INT)");
	conn1->exec("INSERT INTO t1(x) VALUES(1)");
	conn1->exec("INSERT INTO t1(x) VALUES(5)");

	// create another table with single value
	auto conn2 = dbPool->get_conn();
	conn2->exec("CREATE TABLE t2(x INT)");
	conn2->exec("INSERT INTO t2(x) VALUES(1)");

	auto stmt1 = conn1->get_stmt("SELECT * FROM t1 ORDER BY x");
	BOOST_TEST((db_stmt::return_code::row == stmt1->execute()));

	auto stmt2 = conn2->get_stmt("SELECT * FROM t2 ORDER BY x");
	auto thread2 = [&stmt2]() {
	  BOOST_TEST((db_stmt::return_code::row == stmt2->execute()));
	};

	// execute stmt2 while stmt1 still has a selection index and wait for stmt2 to complete; both selection indices
	// should be correct
	std::thread t(thread2);
	t.join();

	// make sure t1 index is still valid
	BOOST_TEST((stmt1->get_int32(0) == 1));
	BOOST_TEST((db_stmt::return_code::row == stmt1->execute()));
	BOOST_TEST((stmt1->get_int32(0) == 5));
	BOOST_TEST((db_stmt::return_code::done == stmt1->execute()));

	// make sure t2 index is still valid
	BOOST_TEST((stmt2->get_int32(0) == 1));
	BOOST_TEST((db_stmt::return_code::done == stmt2->execute()));
}

BOOST_AUTO_TEST_CASE(connection_guard_scope_test)
{
	auto conn = sqlite_conn_pool::create(":memory:")->get_conn();

	// make sure that conn is not invalid once the connection pool shared pointer has died
	BOOST_TEST((db_stmt::return_code::ok == conn->exec("CREATE TABLE t1(x INT)")));
}

BOOST_AUTO_TEST_CASE(statement_scope_test)
{
	std::unique_ptr<db_stmt> stmt;
	{
		auto dbPool = sqlite_conn_pool::create(":memory:");

		// create table with values
		auto conn = dbPool->get_conn();
		conn->exec("CREATE TABLE t1(x INT)");
		conn->exec("INSERT INTO t1(x) VALUES(1)");
		conn->exec("INSERT INTO t1(x) VALUES(5)");

		stmt = conn->get_stmt("SELECT * FROM t1 ORDER BY x");
	}

	// make sure that conn falling out of scope doesn't cause the prepared statement to crash
	BOOST_TEST((db_stmt::return_code::row == stmt->execute()));
	BOOST_TEST((stmt->get_int32(0) == 1));
	BOOST_TEST((db_stmt::return_code::row == stmt->execute()));
	BOOST_TEST((stmt->get_int32(0) == 5));
	BOOST_TEST((db_stmt::return_code::done == stmt->execute()));
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
