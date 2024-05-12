CMAKE_MINIMUM_REQUIRED (VERSION 3.11)
CMAKE_POLICY (VERSION 3.11)

PROJECT (dbpool VERSION 0.0.1.0 LANGUAGES C CXX)

SET (CMAKE_CXX_STANDARD 20)

ADD_LIBRARY (dbpool STATIC "")

TARGET_INCLUDE_DIRECTORIES (dbpool
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include
)

INCLUDE (cmake/date.cmake)
INCLUDE (cmake/fmt.cmake)

TARGET_LINK_LIBRARIES (dbpool
        PUBLIC
        fmt
        pthread

        PRIVATE
        date::date
)

TARGET_SOURCES (dbpool
        PRIVATE
        src/db_conn.cpp
        src/db_conn_pool.cpp
        src/db_stmt.cpp
)

IF (MSVC)
    TARGET_SOURCES (dbpool
            PRIVATE
            src/db_conn.h
            src/db_conn_impl.h
            src/db_conn_pool.h
            src/db_file.h
            src/db_stmt.h
    )
ENDIF ()

OPTION (USE_SQLITE3 "Build SQLite3 code" ON)
IF (USE_SQLITE3)
    FIND_PACKAGE (SQLite3 REQUIRED)

    TARGET_LINK_LIBRARIES (dbpool
            PRIVATE
            ${SQLite3_LIBRARIES}
    )

    TARGET_SOURCES (dbpool
            PRIVATE
            src/sqlite/sqlite_conn_impl.cpp
            src/sqlite/sqlite_conn_pool.cpp
            src/sqlite/sqlite_stmt.cpp
    )

    IF (MSVC)
        TARGET_SOURCES (dbpool
                PRIVATE
                src/sqlite/sqlite_conn_impl.h
                src/sqlite/sqlite_conn_pool.h
                src/sqlite/sqlite_stmt.h
        )
    ENDIF ()
ENDIF ()

OPTION (BUILD_TESTS "Build database tests" ON)
IF (BUILD_TESTS)
    ADD_SUBDIRECTORY (tests)
ENDIF ()
