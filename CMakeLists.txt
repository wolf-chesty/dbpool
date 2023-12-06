cmake_minimum_required (VERSION 3.11)
cmake_policy (VERSION 3.11)

project (dbpool VERSION 0.0.1.0 LANGUAGES C CXX)

add_library(dbpool STATIC "")

target_include_directories(dbpool
	PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/include
)

include (cmake/date.cmake)
include (cmake/fmt.cmake)
#include (cmake/stduuid.cmake)

target_link_libraries(dbpool
	PUBLIC
		fmt
		pthread
#		stduuid

	PRIVATE
		date::date
)

target_sources(dbpool
	PRIVATE
		src/db_conn_guard.cpp
		src/db_conn_pool.cpp
)

if (MSVC)
	target_sources(dbpool
		PRIVATE
			src/db_conn.h
			src/db_conn_guard.h
			src/db_conn_pool.h
			src/db_file.h
			src/db_stmt.h
	)
endif ()

option(DBPOOL_USE_SQLITE3 "Build SQLite3 code" ON)

if (DBPOOL_USE_SQLITE3)
	find_package(SQLite3 REQUIRED)

	target_link_libraries(dbpool
		PRIVATE
			${SQLite3_LIBRARIES}
	)

	target_sources(dbpool
		PRIVATE
			src/sqlite/sqlite_conn.cpp
			src/sqlite/sqlite_conn_pool.cpp
			src/sqlite/sqlite_stmt.cpp
	)

	if (MSVC)
		target_sources(dbpool
			PRIVATE
				src/sqlite/sqlite_conn.h
				src/sqlite/sqlite_conn_pool.h
				src/sqlite/sqlite_stmt.h
		)
	endif ()
endif ()


option(DBPOOL_TESTS "Build database tests" ON)

if (DBPOOL_TESTS)
	add_subdirectory(tests)
endif ()
