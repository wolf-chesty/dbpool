#pragma once

#include <string>
//#include "uuid.h"

class db_stmt {
public:
	enum class return_code {
	  ok,
	  error,
	  row,
	  done
	};

public:
	db_stmt() = default;
	db_stmt(db_stmt&& stmt) = default;
	virtual ~db_stmt() = default;

	db_stmt(const db_stmt&) = delete;
	db_stmt& operator=(const db_stmt&) = delete;

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
};
