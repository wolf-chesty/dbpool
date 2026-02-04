#include "dbpool/sqlite/ConnectionPool.hpp"

#include "dbpool/sqlite/ConnectionPoolImpl.hpp"
#include <cassert>

using namespace dbpool::sqlite;

ConnectionPool::ConnectionPool(std::string_view filename, size_t pool_size,
                               ConnectionPoolImpl::optimization_period_t optimization_period, size_t analysis_limit)
    : dbpool::ConnectionPool<ConnectionPoolImpl>(
          std::make_shared<ConnectionPoolImpl>(filename, pool_size, optimization_period, analysis_limit))
{
}

void ConnectionPool::commit()
{
    auto conn_pool = get_conn_pool();
    assert(conn_pool);
    conn_pool->commit();
}

std::string ConnectionPool::get_filename() const
{
    auto const conn_pool = get_conn_pool();
    assert(conn_pool);
    return conn_pool->get_filename();
}

bool ConnectionPool::is_open() const
{
    auto const conn_pool = get_conn_pool();
    assert(conn_pool);
    return conn_pool->is_open();
}
