#pragma once

#include <string_view>

class db_file {
public:
	virtual ~db_file() = default;

	virtual void commit() = 0;
	virtual std::string_view get_filename() const = 0;
	virtual bool is_open() const = 0;
};
