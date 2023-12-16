# Database connection pooling library
The purpose of this library is to provide a database connection pool.

## Example use of the library
A really basic example follows below where the program will create an in-memory database and then create a table in that
database. This example is really simple since there is only a single thread of execution and very little risk of thread
contention when using the database handle.
```
#include "sqlite/sqlite_conn_pool.h"
#include "db_conn.h"
#include "db_stmt.h"

int main() {
    auto dbPool = sqlite_conn_pool::create(":memory:");

    // get a connection from the pool
    auto conn = dbPool->get_conn();
    
    // execute an SQL statement against the connection
    auto ret = conn.exec("CREATE TABLE t1(x INT)");
}
```
When trying to reuse a database connection across multiple threads of execution you can run into an issue where records
returned from an SQL statement can overwrite an already existing result causing undefined behavior, such as the
following example.
```
#include <thread>
#include "sqlite/sqlite_conn_pool.h"
#include "db_conn.h"
#include "db_stmt.h"
 
void worker_thread(db_conn_guard& conn) {
    conn.exec("SELECT * FROM table1;");
    
    // iterating over the returned records from the above statement can result
    // in bad things since the database connection was shared between two
    // threads of execution
}
 
int main() {
    auto dbPool = sqlite_conn_pool::create(":memory:");

    // get a connection from the pool
    auto conn = dbPool->get_conn();

    // notice that we are sharing conn with the thread; this will cause issues
    std::thread worker(worker_thread, conn);

    conn.exec("SELECT * FROM table2;");
    
    // iterating over the returned records from the above statement can result
    // in bad things since worker_thread can potentially overwrite the return
    // results if the SQL statement in worker_thread is executed after this
    // line of code
}
```
In order to avoid invalidating the returned statements from another database file connection is to use a separate
database connections for each thread of execution. For example, by changing the line 
```std::thread worker(worker_thread, conn)``` to ```std::thread worker(worker_thread, dbPool->get_conn())``` the code
will perform correctly.