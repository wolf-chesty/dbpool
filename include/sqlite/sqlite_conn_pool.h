#pragma once

#include <sqlite3.h>
#include <thread>
#include "db_conn_pool.h"
#include "db_file.h"

class sqlite_conn_pool
		: public db_conn_pool, public db_file, public std::enable_shared_from_this<sqlite_conn_pool> {
private:
	struct _params {
	  std::string_view filename;
	  size_t poolSize;
	  size_t optimizationTimeout;
	};

public:
	sqlite_conn_pool(_params params);
	~sqlite_conn_pool() override;

	static std::shared_ptr<db_conn_pool> create(std::string_view filename, const size_t poolSize = defaultPoolSize,
			const size_t optimizationTimeout = defaultOptimizationTimeout);

	void commit() override;
	std::string_view get_filename() const override;
	bool is_open() const override;

	int64_t get_schema() override;
	void set_schema(const int64_t schema) override;

protected:
	void initialize();

	db_conn* new_conn() override;
	std::shared_ptr<db_conn_pool> shared_base_ptr() override;

private:
	sqlite_conn_pool(std::string_view filename, const size_t poolSize, const size_t optimizationTimeout);

	std::string mFilename;

	void start_optimization_thread(const size_t timeout, const size_t threshold);
	void stop_optimization_thread();
	void optimization_thread(const size_t timeout, const size_t threshold);

	std::unique_ptr<std::thread> mOptimizationThread;
	std::mutex mOptimizationMutex;
	std::condition_variable mOptimizationCondition;
	bool mRunOptimizationThread{true};

	static const size_t defaultPoolSize;
	static const size_t defaultOptimizationTimeout;

	sqlite3* mDb{};
};
