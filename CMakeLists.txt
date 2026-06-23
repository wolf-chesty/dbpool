cmake_minimum_required(VERSION 3.21)
cmake_policy(VERSION 3.21)

project(dbpool VERSION 0.0.0.0 LANGUAGES C CXX)

# ----------------------------------------------------------------------------------------------------------------------
# Options
# ----------------------------------------------------------------------------------------------------------------------
option(DBPOOL_ENABLE_SQLITE3 "Enable SQLite3 support" ON)
option(DBPOOL_UNIT_TESTS "Build unit tests" OFF)

# ----------------------------------------------------------------------------------------------------------------------
# Project dependencies
# ----------------------------------------------------------------------------------------------------------------------
find_package(fmt 12.1.0 REQUIRED)
find_package(date 3.0.0 REQUIRED)

# ----------------------------------------------------------------------------------------------------------------------
# Project definition
# ----------------------------------------------------------------------------------------------------------------------
add_library(dbpool OBJECT "")

target_compile_features(dbpool PRIVATE cxx_std_20)

target_include_directories(dbpool
PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/include
)

target_link_libraries(dbpool
PUBLIC
    fmt
    pthread

PRIVATE
    date::date
)

target_sources(dbpool
PRIVATE
    src/impl/ConnectionPoolImpl.cpp
    src/util/PooledConnection.cpp
    src/Connection.cpp
    src/PreparedStmt.cpp
)

if(DBPOOL_ENABLE_SQLITE3)
    find_package(SQLite3 REQUIRED)

    target_link_libraries(dbpool PRIVATE ${SQLite3_LIBRARIES})

    target_sources(dbpool
    PRIVATE
        src/sqlite/ConnectionImpl.cpp
        src/sqlite/ConnectionPool.cpp
        src/sqlite/ConnectionPoolImpl.cpp
        src/sqlite/PreparedStmtImpl.cpp
    )
endif()

#if(DBPOOL_ENABLE_MARIADB)
#    pkg_check_modules(mariadb REQUIRED IMPORTED_TARGET "mysqlclient")
#
#    target_link_libraries(dbpool PRIVATE PkgConfig::mariadb)
#endif()

add_subdirectory(tests)
