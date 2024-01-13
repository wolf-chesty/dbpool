#pragma once

#include <memory>
#include <string>
//#include "uuid.h"

class db_conn_guard;

//!
//! \class db_stmt
//! \brief Interface for a database prepared statement.
//!
//! The responsibility of this class is to provide a standard interface for inter-oping with prepared statements from
//! different database APIs.
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
	db_stmt(db_stmt const&) = delete;
	db_stmt(db_stmt&&) = delete;
	explicit db_stmt(std::shared_ptr<db_conn_guard> conn);

	virtual ~db_stmt() = default;

	db_stmt& operator=(db_stmt const&) = delete;
	db_stmt& operator=(db_stmt&&) = delete;

	//!
	//! \brief Executes the prepared statement.
	//!
	//! \return \c return_code for the statement execution.
	//!
	virtual return_code execute() = 0;

	virtual void bind_blob(int32_t const index, void const* data, size_t const nbytes) = 0;
	virtual void bind_bool(int32_t const index, bool const value) = 0;
	virtual void bind_date(int32_t const index, std::string_view date) = 0;
	virtual void bind_double(int32_t const index, double const value) = 0;
	virtual void bind_int32(int32_t const index, int32_t const value) = 0;
	virtual void bind_int64(int32_t const index, int64_t const value) = 0;
	virtual void bind_null(int32_t const index) = 0;
//	virtual void bind_uuid(int32_t const index, uuids::uuid& const id) = 0;
//	virtual void bind_uuid(int32_t const index, uuids::uuid&& id) = 0;
	virtual void bind_text(int32_t const index, std::string_view text) = 0;

	virtual bool get_bool(int32_t const index) = 0;
	virtual std::string get_date(int32_t const index) = 0;
	virtual double get_double(int32_t const index) = 0;
	virtual int32_t get_int32(int32_t const index) = 0;
	virtual int64_t get_int64(int32_t const index) = 0;
	virtual std::string get_text(int32_t const index) = 0;
//	virtual uuids::uuid get_uuid(int32_t const index) = 0;

private:
	// nothing is really done with this mConn member; this is just here to make sure that the db_conn_guard object that
	// created this prepared statement isn't destroyed and the database connection isn't returned to the connection pool
	// before the current thread is done using this prepared statement
	std::shared_ptr<db_conn_guard> mConn;
};
