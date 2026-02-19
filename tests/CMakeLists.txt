add_executable (dbpool_unit_tests "")

enable_testing ()

if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.30.0")
    cmake_policy (SET CMP0167 NEW)
endif ()

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
    Boost::system
    Boost::filesystem
    Boost::unit_test_framework
    dbpool
    uuid
)

target_sources (dbpool_unit_tests
    PRIVATE
    src/main.cpp

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
