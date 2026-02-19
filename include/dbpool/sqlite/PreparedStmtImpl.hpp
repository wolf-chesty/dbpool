// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#ifndef DBPOOL_SQLITE_PREPARED_STATEMENT_HPP
#define DBPOOL_SQLITE_PREPARED_STATEMENT_HPP

#include "dbpool/impl/PreparedStmtImpl.hpp"

#include <span>
#include <sqlite3.h>
#include <string_view>

namespace dbpool::sqlite {

///
/// @class PreparedStmtImpl
///
/// @brief Implements the \c PreparedStmt interface for an SQL prepared statement.
///
class PreparedStmtImpl : public dbpool::PreparedStmtImpl {
public:
    PreparedStmtImpl() = delete;
    PreparedStmtImpl(PreparedStmtImpl const &) = delete;
    PreparedStmtImpl(PreparedStmtImpl &&) noexcept = delete;

    /// @brief ctor for the object.
    ///
    /// @param db Pointer to a sqlite3 database pointer that this prepared statement was created against.
    /// @param sql SQL statement.
    explicit PreparedStmtImpl(sqlite3 *db, std::string_view sql);

    /// @brief dtor for the object.
    ///
    /// Resets the prepared statement (frees any memory being used) for later re-use.
    ~PreparedStmtImpl() override;

    PreparedStmtImpl &operator=(PreparedStmtImpl const &) = delete;
    PreparedStmtImpl &operator=(PreparedStmtImpl &&) noexcept = delete;

    /// @brief Executes the prepared statement.
    ///
    /// @return Return code from executing the prepared statement.
    ReturnCode execute() override;

    /// @brief Resets the prepared statement for reuse.
    ///
    /// @return Return code from resetting the prepared statement.
    PreparedStmtImpl::ReturnCode reset() override;

    /// @brief Returns \c true if the column is null.
    ///
    /// @param index Index of the column to bind \c value to.
    ///
    /// @return \c true if the column is null.
    bool is_null(int32_t const index) override;

    /// @brief Binds memory to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    void bind_blob(int32_t const index, std::span<std::byte const> const &value) override;

    /// @brief Binds a boolean value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    void bind_bool(int32_t const index, bool const value) override;

    /// @brief Binds a date value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    void bind_date(int32_t const index, std::string_view value) override;

    /// @brief Binds a double value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    void bind_double(int32_t const index, double const value) override;

    /// @brief Binds an in32_t value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    void bind_int32(int32_t const index, int32_t const value) override;

    /// @brief Binds an in64_t value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    void bind_int64(int32_t const index, int64_t const value) override;

    /// @brief Sets a prepared statement field to NULL.
    ///
    /// @param index Index of the column to bind \c value to.
    void bind_null(int32_t const index) override;

    /// @brief Binds a string value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    void bind_text(int32_t const index, std::string_view value) override;

    /// @brief Binds a UTF-16 string value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    void bind_text16(int32_t const index, std::u16string_view value) override;

    /// @brief Binds a UUID value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    void bind_uuid(int32_t const index, std::span<std::byte const> const &value) override;

    /// @brief Returns binary data from a prepared statement result.
    ///
    /// @return Binary data from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    std::vector<std::byte> get_blob(int32_t index) override;

    /// @brief Returns a \c bool value from a prepared statement result.
    ///
    /// @return \c bool value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    bool get_bool(int32_t const index) override;

    /// @brief Returns a date value from a prepared statement result.
    ///
    /// @return Date value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    std::string get_date(int32_t const index) override;

    /// @brief Returns a \c double value from a prepared statement result.
    ///
    /// @return \c double value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    double get_double(int32_t const index) override;

    /// @brief Returns an \c int32_t value from a prepared statement result.
    ///
    /// @return \c int32_t value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    int32_t get_int32(int32_t const index) override;

    /// @brief Returns an \c int64_t value from a prepared statement result.
    ///
    /// @return \c int64_t value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    int64_t get_int64(int32_t const index) override;

    /// @brief Returns a text value from a prepared statement result.
    ///
    /// @return Text value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    std::string get_text(int32_t const index) override;

    /// @brief Returns a UTF-16 text value from a prepared statement result.
    ///
    /// @return UTF-16 text value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    std::u16string get_text16(int32_t const index) override;

    /// @brief Returns a UUID value from a prepared statement result.
    ///
    /// @return UUID value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    std::array<std::byte, 16> get_uuid(int32_t const index) override;

    /// @brief Converts sqlite3 specific error codes to library specific error codes.
    ///
    /// @return Library specific error code.
    ///
    /// @param code sqlite3 error code to convert.
    static ReturnCode to_error_code(int code);

private:
    sqlite3 *db_{};        ///< Pointer to sqlite3 database.
    sqlite3_stmt *stmt_{}; ///< Pointer to sqlite3 prepared statement.
};

} // namespace dbpool::sqlite

#endif
