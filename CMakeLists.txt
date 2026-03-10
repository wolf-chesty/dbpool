cmake_minimum_required (VERSION 3.11)
cmake_policy (VERSION 3.11)

project (dbpool VERSION 0.0.0.0 LANGUAGES C CXX)

set (CMAKE_CXX_STANDARD 20)

add_library (dbpool OBJECT "")

option (DBPOOL_ENABLE_SQLITE3 "Enable SQLite3 support" ON)
option (DBPOOL_BUILD_TESTS "Build tests" ON)
option (DBPOOL_COVERAGE "Enable GCC Coverage" OFF)
option (DBPOOL_SANITIZE_ADDRESS "Enable GCC Address Sanitizer (ASan)" OFF)
option (DBPOOL_SANITIZE_THREAD "Enable GCC Thread Sanitizer (TSan)" OFF)

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
    src/impl/ConnectionPoolImpl.cpp
    src/util/PooledConnection.cpp
    src/Connection.cpp
    src/PreparedStmt.cpp
)

if (DBPOOL_ENABLE_SQLITE3)
    find_package (SQLite3 REQUIRED)

    target_link_libraries (dbpool PRIVATE ${SQLite3_LIBRARIES})

    target_sources (dbpool
        PRIVATE
        src/sqlite/ConnectionImpl.cpp
        src/sqlite/ConnectionPool.cpp
        src/sqlite/ConnectionPoolImpl.cpp
        src/sqlite/PreparedStmtImpl.cpp
    )
endif ()

if (DBPOOL_ENABLE_MARIADB)
    pkg_check_modules (mariadb REQUIRED IMPORTED_TARGET "mysqlclient")

    target_link_libraries (dbpool PRIVATE PkgConfig::mariadb)
endif ()

if (DBPOOL_COVERAGE)
    target_compile_options (dbpool PRIVATE -O0 --coverage -g)
    target_link_options (dbpool PRIVATE --coverage)
    target_link_libraries (dbpool PRIVATE gcov)
elseif (DBPOOL_SANITIZE_ADDRESS)
    target_compile_options (dbpool PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer -g)
    target_link_options (dbpool PRIVATE -fsanitize=address,undefined)
elseif (DBPOOL_SANITIZE_THREAD)
    target_compile_options (dbpool PRIVATE -fsanitize=thread -fno-omit-frame-pointer -g)
    target_link_options (dbpool PRIVATE -fsanitize=thread)
endif ()

if (DBPOOL_BUILD_TESTS)
    include (cmake/googletest.cmake)
    add_subdirectory (tests)
endif ()
