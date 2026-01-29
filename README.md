# Database connection pooling library

The purpose of this library is to provide a database connection pool.

## Example use of the library

A really basic example follows below where the program will create an in-memory database and then create a table in that
database. This example is really simple since there is only a single thread of execution and very little risk of thread
contention when using the database handle.

```
#include "dbpool/sqlite/ConnectionPool.hpp"
#include "dbpool/Connection.hpp"
#include "dbpool/PreparedStmt.hpp"

int main() {
    auto dbPool = dbpool::sqlite::ConnectionPool::create(":memory:");

    // Get a connection from the pool
    auto conn = dbPool->get_conn();
    
    // Execute an SQL statement against the connection
    auto ret = conn->exec("CREATE TABLE t1(x INT)");
}
```

When trying to reuse a database connection across multiple threads of execution you can run into an issue where records
returned from an SQL statement can overwrite an already existing result (in another thread) causing undefined behavior
(see item 2 [here](https://www.sqlite.org/threadsafe.html)), such as in the following example.

```
#include <thread>
#include "dbpool/sqlite/ConnectionPool.hpp"
#include "dbpool/Connection.hpp"
#include "dbpool/PreparedStmt.hpp"
 
void worker_thread(std::shared_ptr<dbpool::Connection> const &conn) {
    conn->exec("SELECT * FROM table1;");
    
    // iterating over the returned records from conn can result in bad things
    // since the database connection was shared between two threads of execution
}
 
int main() {
    auto dbPool = dbpool::sqlite::ConnectionPool::create(":memory:");

    // Get a connection from the pool
    auto conn = dbPool->get_conn();

    // Notice that we are sharing conn with the thread; this will cause issues
    std::thread worker(worker_thread, conn);

    conn->exec("SELECT * FROM table2;");
    
    // Since the connection is shared across threads the results from worker_thread can
    // overwrite the query results for this thread (or vice-versa). In order to avoid this
    // each thread should have their own pointer to the database.
}
```

In order to avoid invalidating the returned record from contending threads you can do the following.

```
#include <thread>
#include "dbpool/sqlite/ConnectionPool.hpp"
#include "dbpool/Connection.hpp"
#include "dbpool/PreparedStmt.hpp"
 
void worker_thread(std::shared_ptr<dbpool::Connection> const &conn) {
    conn->exec("SELECT * FROM table1;");
}
 
int main() {
    auto dbPool = dbpool::sqlite::Connection::create(":memory:");

    // Get a connection from the pool
    auto conn = dbPool->get_conn();

    // Give the thread its own connection to work with
    std::thread worker(worker_thread, dbPool->get_conn());

    conn->exec("SELECT * FROM table2;");
    
    // Since work_thread and this thread of execution have their own pointer to the
    // underlying database the returned results will not overwrite each other.
}
```
