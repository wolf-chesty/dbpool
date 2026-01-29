#ifndef DBPOOL_PREPARED_STATEMENT_HPP
#define DBPOOL_PREPARED_STATEMENT_HPP

#include <memory>
#include <span>
#include <string>
#include <vector>

namespace dbpool {

class Connection;

//!
//! \class PreparedStmt
//! \brief Interface for a database prepared statement.
//!
//! The responsibility of this class is to provide a standard interface for inter-oping with prepared statements from
//! different database APIs.
//!
class PreparedStmt {
public:
    enum class return_code { ok, error, row, done };

public:
    PreparedStmt(PreparedStmt const &) = delete;
    PreparedStmt(PreparedStmt &&) noexcept = default;
    explicit PreparedStmt(std::shared_ptr<Connection> conn) noexcept;

    virtual ~PreparedStmt() = default;

    PreparedStmt &operator=(PreparedStmt const &) = delete;
    PreparedStmt &operator=(PreparedStmt &&) noexcept = default;

    std::shared_ptr<Connection> get_conn();

    //!
    //! \brief Executes the prepared statement.
    //!
    //! \return \c return_code for the statement execution.
    //!
    virtual return_code execute() = 0;

    virtual void bind_blob(int32_t const index, std::span<std::byte const> const &value) = 0;
    virtual void bind_bool(int32_t const index, bool const value) = 0;
    virtual void bind_date(int32_t const index, std::string_view date) = 0;
    virtual void bind_double(int32_t const index, double const value) = 0;
    virtual void bind_int32(int32_t const index, int32_t const value) = 0;
    virtual void bind_int64(int32_t const index, int64_t const value) = 0;
    virtual void bind_null(int32_t const index) = 0;
    virtual void bind_text(int32_t const index, std::string_view text) = 0;
    virtual void bind_text16(int32_t const index, std::u16string_view text) = 0;
    virtual void bind_uuid(int32_t const index, std::span<std::byte const> const &value) = 0;

    virtual std::vector<std::byte> get_blob(int32_t index) = 0;
    virtual bool get_bool(int32_t const index) = 0;
    virtual std::string get_date(int32_t const index) = 0;
    virtual double get_double(int32_t const index) = 0;
    virtual int32_t get_int32(int32_t const index) = 0;
    virtual int64_t get_int64(int32_t const index) = 0;
    virtual std::string get_text(int32_t const index) = 0;
    virtual std::u16string get_text16(int32_t const index) = 0;
    virtual std::array<uint8_t, 16> get_uuid(int32_t const index) = 0;

private:
    // Nothing is really done with this mConn member; this is just here to make sure that the db_conn object that
    // created this prepared statement isn't destroyed and the database connection isn't prematurely returned to the
    // connection pool before the current thread is done using this prepared statement
    std::shared_ptr<Connection> m_conn;
};

} // namespace dbpool

#endif
