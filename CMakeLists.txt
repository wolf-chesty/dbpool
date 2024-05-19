cmake_minimum_required (VERSION 3.11)
cmake_policy (VERSION 3.11)

project (dbpool VERSION 0.0.1.0 LANGUAGES C CXX)

set (CMAKE_CXX_STANDARD 20)

add_library (dbpool STATIC "")

target_include_directories (dbpool
    PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)

include (cmake/date.cmake)
include (cmake/fmt.cmake)

target_link_libraries (dbpool
    PUBLIC
    fmt
    pthread

    PRIVATE
    date::date
)

target_sources (dbpool
    PRIVATE
    src/db_conn.cpp
    src/db_conn_pool.cpp
    src/db_stmt.cpp
)

option (USE_SQLITE3 "Build SQLite3 code" ON)
if (USE_SQLITE3)
    find_package (SQLite3 REQUIRED)

    target_link_libraries (dbpool
        PRIVATE
        ${SQLite3_LIBRARIES}
    )

    target_sources (dbpool
        PRIVATE
        src/sqlite/sqlite_conn_impl.cpp
        src/sqlite/sqlite_conn_pool.cpp
        src/sqlite/sqlite_stmt.cpp
    )
endif ()

option (BUILD_TESTS "Build database tests" ON)
if (BUILD_TESTS)
    add_subdirectory (tests)
endif ()
