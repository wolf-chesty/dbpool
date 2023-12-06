#pragma once

#include <condition_variable>
#include <forward_list>
#include <mutex>
#include <string>
#include "db_conn_guard.h"

class db_conn;

class db_conn_pool {
	friend db_conn_guard;

public:
	using conn_cache_type = std::forward_list<db_conn*>;

public:
	db_conn_pool(const size_t poolSize = 5);
	virtual ~db_conn_pool();

	db_conn_guard get_conn();

	virtual int64_t get_schema() = 0;
	virtual void set_schema(int64_t schema) = 0;

	void set_prep_sql(std::string_view sql);

protected:
	db_conn* pop_conn();
	void push_conn(db_conn* conn);

	virtual db_conn* new_conn() = 0;
	virtual std::shared_ptr<db_conn_pool> shared_base_ptr() = 0;

	virtual void prep_conn(db_conn*);

private:
	std::condition_variable mConnectionCondition;
	std::mutex mConnectionMutex;
	conn_cache_type mInUseConnections;
	conn_cache_type mAvailableConnections;

	std::string_view mPrepSql;
};
