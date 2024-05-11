#pragma once

#include <condition_variable>
#include <forward_list>
#include <memory>
#include <mutex>
#include <string>

#include "db_conn.h"

class db_conn_impl;

//!
//! \class db_conn_pool
//! \brief A database connection pool class.
//!
//! The responsibility of this class is to manage (create and destroy) \c db_conn objects that will
//! be used by the application to communicate with the database. This class will return \c
//! db_conn objects to consumers of this library.
//!
//! Classes that derive from this will need to implement the API specific code required to return a
//! \c db_conn object that wraps a API specific database handle.
//!
class db_conn_pool {
    friend db_conn;

public:
    using conn_cache_type = std::forward_list<std::unique_ptr<db_conn_impl>>;

public:
    db_conn_pool(db_conn_pool const &) = delete;
    db_conn_pool(db_conn_pool &&) = delete;
    explicit db_conn_pool(size_t const poolSize = 5);

    virtual ~db_conn_pool() = default;

    db_conn_pool &operator=(db_conn_pool const &) = delete;
    db_conn_pool &operator=(db_conn_pool &&) = delete;

    std::shared_ptr<db_conn> get_conn();

    virtual int64_t get_schema() = 0;
    virtual void set_schema(int64_t schema) = 0;

    void set_prep_sql(std::string_view sql);

protected:
    std::unique_ptr<db_conn_impl> pop_conn();
    void push_conn(std::unique_ptr<db_conn_impl> conn);

    //!
    //! \brief Function to create a new \c db_conn object to keep in the connection pool.
    //!
    //! \return A \c db_conn object on the stack.
    //!
    virtual std::unique_ptr<db_conn_impl> new_conn() = 0;

    //!
    //! \brief Returns an \c std::shared_ptr to this object.
    //!
    //! \return \c std::shared_ptr to this object.
    //!
    //! Functions in this class will use managed pointers to the base class (this object) of derived classes. Most
    //! objects in this library are coded against the \c db_conn_pool interface and some objects (such as the \c db_conn
    //! class) use shared pointers to the connection pool in order to control the lifetime of the connection pool. By
    //! using managed pointers we can avoid prematurely closing the database connections in the pool by having each
    //! \c db_conn keep a \c std::shared_ptr to the owning connection pool.
    //!
    virtual std::shared_ptr<db_conn_pool> shared_base_ptr() = 0;

    virtual void prep_conn(db_conn_impl &);

private:
    std::condition_variable mConnectionCondition;
    std::mutex mConnectionMutex;
    conn_cache_type mAvailableConnections;

    std::string mPrepSql;
};
