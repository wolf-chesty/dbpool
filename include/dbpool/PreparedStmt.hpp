// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#ifndef DBPOOL_PREPARED_STATEMENT_HPP
#define DBPOOL_PREPARED_STATEMENT_HPP

#include "dbpool/PreparedStmtImpl.hpp"
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace dbpool {

class PooledConnection;
class PreparedStmtImpl;

///
/// @class PreparedStmt
///
/// @brief Interface for a database prepared statement.
///
class PreparedStmt {
public:
    using ReturnCode = dbpool::PreparedStmtImpl::ReturnCode;

public:
    PreparedStmt() = default;
    PreparedStmt(PreparedStmt const &) = delete;
    PreparedStmt(PreparedStmt &&) noexcept = default;

    /// @brief Creates a prepared statement wrapping impl.
    ///
    /// @param pooled_conn Pointer to a scoped connection.
    /// @param impl Prepared statement implementation.
    PreparedStmt(std::shared_ptr<PooledConnection> pooled_conn, std::unique_ptr<PreparedStmtImpl> impl) noexcept;

    virtual ~PreparedStmt();

    PreparedStmt &operator=(PreparedStmt const &) = delete;
    PreparedStmt &operator=(PreparedStmt &&) noexcept = default;

    /// @brief Executes the prepared statement.
    ///
    /// @return \c return_code for the statement execution.
    ReturnCode execute();

    /// @brief Binds memory to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    void bind_blob(int32_t const index, std::span<std::byte const> const &value);

    /// @brief Binds a boolean value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    void bind_bool(int32_t const index, bool const value);

    /// @brief Binds a date value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    void bind_date(int32_t const index, std::string_view value);

    /// @brief Binds a double value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    void bind_double(int32_t const index, double const value);

    /// @brief Binds an in32_t value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    void bind_int32(int32_t const index, int32_t const value);

    /// @brief Binds an in64_t value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    void bind_int64(int32_t const index, int64_t const value);

    /// @brief Sets a prepared statement field to NULL.
    ///
    /// @param index Index of the column to bind \c value to.
    void bind_null(int32_t const index);

    /// @brief Binds a string value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    void bind_text(int32_t const index, std::string_view value);

    /// @brief Binds a UTF-16 string value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    void bind_text16(int32_t const index, std::u16string_view value);

    /// @brief Binds a UUID value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    void bind_uuid(int32_t const index, std::span<std::byte const> const &value);

    /// @brief Returns binary data from a prepared statement result.
    ///
    /// @return Binary data from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    std::vector<std::byte> get_blob(int32_t const index);

    /// @brief Returns a \c bool value from a prepared statement result.
    ///
    /// @return \c bool value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    bool get_bool(int32_t const index);

    /// @brief Returns a date value from a prepared statement result.
    ///
    /// @return Date value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    std::string get_date(int32_t const index);

    /// @brief Returns a \c double value from a prepared statement result.
    ///
    /// @return \c double value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    double get_double(int32_t const index);

    /// @brief Returns an \c int32_t value from a prepared statement result.
    ///
    /// @return \c int32_t value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    int32_t get_int32(int32_t const index);

    /// @brief Returns an \c int64_t value from a prepared statement result.
    ///
    /// @return \c int64_t value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    int64_t get_int64(int32_t const index);

    /// @brief Returns a text value from a prepared statement result.
    ///
    /// @return Text value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    std::string get_text(int32_t const index);

    /// @brief Returns a UTF-16 text value from a prepared statement result.
    ///
    /// @return UTF-16 text value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    std::u16string get_text16(int32_t const index);

    /// @brief Returns a UUID value from a prepared statement result.
    ///
    /// @return UUID value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    std::array<uint8_t, 16> get_uuid(int32_t const index);

private:
    std::shared_ptr<PooledConnection> pooled_conn_; ///< Pointer to a scoped connection.
    std::unique_ptr<PreparedStmtImpl> impl_;        ///< Pointer to the prepared statement implementation.
};

} // namespace dbpool

#endif
