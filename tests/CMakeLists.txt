find_package(GTest)

include(GoogleTest)

add_executable(dbpool_unit_tests "")

target_compile_features(dbpool_unit_tests PRIVATE cxx_std_20)

enable_testing()

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.30.0")
    cmake_policy (SET CMP0167 NEW)
endif()

add_test(NAME DBPoolUnitTests COMMAND dbpool_unit_tests)

target_link_libraries(dbpool_unit_tests
PRIVATE
    $<$<BOOL:${DBPOOL_COVERAGE}>:gcov>

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
