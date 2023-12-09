#pragma once

#include <memory>
#include <string>
//#include "uuid.h"

class db_conn_guard;

//!
//! \class db_stmt
//!
//! \brief Interface for a database prepared statement.
//!
//! The purpose of this class is to provide a layer of abstraction for the different database API methods for statement
//! construction. By coding the rest of the application to this interface the developer can potentially replace the
//! backing database without having to rewrite code.
//!
class db_stmt {
public:
	enum class return_code {
	  ok,
	  error,
	  row,
	  done
	};

public:
	db_stmt(const db_stmt&) = delete;
	db_stmt(db_stmt&&) = delete;
	explicit db_stmt(std::shared_ptr<db_conn_guard> conn);

	virtual ~db_stmt() = default;

	db_stmt& operator=(const db_stmt&) = delete;
	db_stmt& operator=(db_stmt&&) = delete;

	//!
	//! \brief Executes the prepared statement.
	//!
	//! \return \c return_code for the statement execution.
	//!
	virtual return_code execute() = 0;

	virtual void bind_blob(const int32_t index, const void* data, const size_t nbytes) = 0;
	virtual void bind_bool(const int32_t index, const bool value) = 0;
	virtual void bind_date(const int32_t index, std::string_view date) = 0;
	virtual void bind_double(const int32_t index, const double value) = 0;
	virtual void bind_int32(const int32_t index, const int32_t value) = 0;
	virtual void bind_int64(const int32_t index, const int64_t value) = 0;
	virtual void bind_null(const int32_t index) = 0;
//	virtual void bind_uuid(const int32_t index, const uuids::uuid& id) = 0;
//	virtual void bind_uuid(const int32_t index, uuids::uuid&& id) = 0;
	virtual void bind_text(const int32_t index, std::string_view text) = 0;

	virtual bool get_bool(const int32_t index) = 0;
	virtual std::string get_date(const int32_t index) = 0;
	virtual double get_double(const int32_t index) = 0;
	virtual int32_t get_int32(const int32_t index) = 0;
	virtual int64_t get_int64(const int32_t index) = 0;
	virtual std::string get_text(const int32_t index) = 0;
//	virtual uuids::uuid get_uuid(const int32_t index) = 0;

private:
	// nothing is really done with the mConn member; this is just here to make sure that the mConn object that created
	// this prepared statement doesn't go out of scope before the prepared statement, causing the database connection
	// to be returned to the pool for reuse
	std::shared_ptr<db_conn_guard> mConn;
};
