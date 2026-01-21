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
)

target_sources (dbpool_unit_tests
    PRIVATE
    main.cpp
    dbpool_tests.cpp
)
