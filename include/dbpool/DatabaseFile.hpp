// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#ifndef DBPOOL_DATABASE_FILE_HPP
#define DBPOOL_DATABASE_FILE_HPP

#include <string>

namespace dbpool {
/// @class DatabaseFile
///
/// @brief Provides an interface for databases that are file backed (i.e., SQLite3).
class DatabaseFile {
public:
    virtual ~DatabaseFile() = default;

    /// @brief Flushes database changes from memory to disk.
    virtual void commit() = 0;

    /// @brief Returns the database filename.
    ///
    /// @return Database filename.
    virtual std::string getFilename() const = 0;

    /// @brief Returns \c true if the database is open.
    ///
    /// @return \c true if the database is open.
    virtual bool isOpen() const = 0;
};
} // namespace dbpool

#endif
