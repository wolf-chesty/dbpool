add_executable (dbpool_unit_tests "")

enable_testing ()

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.30.0")
    cmake_policy (SET CMP0167 NEW)
endif ()

option (DBPOOL_TEST_COVERAGE "Enable GCC Coverage" OFF)
option (DBPOOL_TEST_SANITIZE_ADDRESS "Enable GCC Address Sanitizer (ASan)" OFF)
option (DBPOOL_TESt_SANITIZE_THREAD "Enable GCC Thread Sanitizer (TSan)" OFF)

find_package (Boost
    COMPONENTS
    filesystem
    system
    unit_test_framework
    REQUIRED
)

add_test (NAME UnitTests COMMAND dbpool_unit_tests)

target_link_libraries (dbpool_unit_tests
    PRIVATE
    gtest_main
    dbpool
    uuid
)

if (DBPOOL_ENABLE_SQLITE3)
    target_sources (dbpool_unit_tests
        PRIVATE
        src/sqlite/file_tests.cpp
        src/sqlite/schema_tests.cpp
        src/sqlite/scope_tests.cpp
        src/sqlite/sqlite_tests.cpp
        src/sqlite/stmt_blob_tests.cpp
        src/sqlite/stmt_bool_tests.cpp
        src/sqlite/stmt_date_tests.cpp
        src/sqlite/stmt_double_tests.cpp
        src/sqlite/stmt_int32_tests.cpp
        src/sqlite/stmt_int64_tests.cpp
        src/sqlite/stmt_null_tests.cpp
        src/sqlite/stmt_text16_tests.cpp
        src/sqlite/stmt_text_tests.cpp
        src/sqlite/stmt_uuid_tests.cpp
    )
endif ()

if (DBPOOL_TEST_COVERAGE)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 --coverage -g")
endif ()

if (DBPOOL_TEST_SANITIZE_ADDRESS)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,undefined -fno-omit-frame-pointer -g")
endif ()

if (DBPOOL_TEST_SANITIZE_THREAD)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -fno-omit-frame-pointer -g")
endif ()
