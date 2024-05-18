#pragma once

#include <string_view>

namespace dbpool {

//!
//! \class db_file
//! \brief Provides an interface for databases that are file backed (i.e., SQLite3).
//!
class db_file {
public:
    virtual ~db_file() = default;

    //!
    //! \brief Flushes database changes from memory to disk.
    //!
    virtual void commit() = 0;

    //!
    //! \brief Returns the database filename.
    //!
    //! \return Database filename.
    //!
    virtual std::string get_filename() const = 0;

    //!
    //! Returns true if the database is open.
    //!
    //! \return \c true if the database is open.
    //!
    virtual bool is_open() const = 0;
};

} // namespace dbpool
