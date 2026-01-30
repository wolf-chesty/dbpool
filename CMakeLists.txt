cmake_minimum_required (VERSION 3.11)
cmake_policy (VERSION 3.11)

project (dbpool VERSION 0.0.0.0 LANGUAGES C CXX)

set (CMAKE_CXX_STANDARD 20)

add_library (dbpool OBJECT "")

option (DBPOOL_ENABLE_SQLITE3 "Enable SQLite3 support" ON)
option (DBPOOL_BUILD_TESTS "Build tests" ON)
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
    src/Connection.cpp
    src/ConnectionPool.cpp
    src/PreparedStmt.cpp
)

if (DBPOOL_ENABLE_SQLITE3)
    find_package (SQLite3 REQUIRED)

    target_link_libraries (dbpool
        PRIVATE
        ${SQLite3_LIBRARIES}
    )

    target_sources (dbpool
        PRIVATE
        src/sqlite/ConnectionImpl.cpp
        src/sqlite/ConnectionPool.cpp
        src/sqlite/PreparedStmt.cpp
    )
endif ()

if (DBPOOL_SANITIZE_ADDRESS)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -fno-omit-frame-pointer -g")
endif ()

if (DBPOOL_SANITIZE_THREAD)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -fno-omit-frame-pointer -g")
endif ()

if (DBPOOL_BUILD_TESTS)
    add_subdirectory (tests)
endif ()
