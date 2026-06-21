find_package(GTest)

include(GoogleTest)

add_executable(dbpool_unit_tests "")

enable_testing()

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.30.0")
    cmake_policy (SET CMP0167 NEW)
endif()

add_test(NAME DBPoolUnitTests COMMAND dbpool_unit_tests)

target_link_libraries(dbpool_unit_tests
PRIVATE
    GTest::gtest_main
    dbpool
    uuid
)

if(DBPOOL_ENABLE_SQLITE3)
    target_sources(dbpool_unit_tests
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
endif()

if(DBPOOL_COVERAGE)
    target_link_options(dbpool_unit_tests PRIVATE --coverage)
    target_link_libraries(dbpool_unit_tests PRIVATE gcov)
elseif(DBPOOL_ASAN)
    target_link_options(dbpool_unit_tests PRIVATE -fsanitize=address,undefined)
elseif(DBPOOL_TSAN)
    target_link_options(dbpool_unit_tests PRIVATE -fsanitize=thread)
elseif(DBPOOL_PERF)
    target_compile_options(dbpool_unit_tests PRIVATE -O2 -fno-omit-frame-pointer -g)
endif()
