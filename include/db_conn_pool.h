#pragma once

#include <condition_variable>
#include <forward_list>
#include <mutex>
#include <string>
#include "db_conn_guard.h"

class db_conn;

//!
//! \class db_conn_pool
//!
//! \brief Provides an interface for a database connection pool class.
//!
//! Different threads of execution cannot use the same database connection when interacting with the same database
//! otherwise query results can be stomped on or crashes can occur. Typically concurrent access to database resources
//! are handled using multiple database connections to the same database resource. The database driver will then
//! coordinate the threaded access of the database resource. Unfortunately, having an unlimited number of database
//! connections can be detrimental to the performance of your application so it's considered good practice to limit the
//! number of concurrent open database connections to a database in order to avoid issues. This class manages a pool of
//! of database connections that will be shared amongst the threads of a multi-threaded program.
//!
class db_conn_pool {
	friend db_conn_guard;

public:
	using conn_cache_type = std::forward_list<db_conn*>;

public:
	db_conn_pool(const db_conn_pool&) = delete;
	db_conn_pool(db_conn_pool&&) = delete;
	explicit db_conn_pool(const size_t poolSize = 5);

	virtual ~db_conn_pool();

	db_conn_pool& operator=(const db_conn_pool&) = delete;
	db_conn_pool& operator=(db_conn_pool&&) = delete;

	std::shared_ptr<db_conn_guard> get_conn();

	virtual int64_t get_schema() = 0;
	virtual void set_schema(int64_t schema) = 0;

	void set_prep_sql(std::string_view sql);

protected:
	db_conn* pop_conn();
	void push_conn(db_conn* conn);

	//!
	//! \brief Function to create a new \c db_conn object to keep in the connection pool.
	//!
	//! \return A \c db_conn object on the stack.
	//!
	virtual db_conn* new_conn() = 0;

	//!
	//! \brief Returns an \c std::shared_ptr to this object.
	//!
	//! \return \c std::shared_ptr to this object.
	//!
	//! Functions in this class will use managed pointers to the base class (this object) of derived classes. Most
	//! objects in this library are coded against the \c db_conn_pool interface and some objects (such as the
	//! \c db_conn_guard class) use shared pointers to the connection pool in order to control the lifetime of the
	//! connection pool. By using managed pointers we can avoid prematurely closing the database connections in the pool
	//! by having each \c db_conn_guard keep a \c std::shared_ptr to the owning connection pool.
	//!
	virtual std::shared_ptr<db_conn_pool> shared_base_ptr() = 0;

	virtual void prep_conn(db_conn*);

private:
	std::condition_variable mConnectionCondition;
	std::mutex mConnectionMutex;
	conn_cache_type mAvailableConnections;

	std::string_view mPrepSql;
};
