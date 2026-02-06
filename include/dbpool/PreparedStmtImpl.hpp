#ifndef DBPOOL_PREPARED_STMT_IMPL_HPP
#define DBPOOL_PREPARED_STMT_IMPL_HPP

#include <array>
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace dbpool {

class PreparedStmtImpl {
public:
    enum class ReturnCode { ok, error, row, done };

public:
    PreparedStmtImpl() = default;
    PreparedStmtImpl(PreparedStmtImpl const &) = delete;
    PreparedStmtImpl(PreparedStmtImpl &&) noexcept = default;
    virtual ~PreparedStmtImpl() = default;

    PreparedStmtImpl &operator=(PreparedStmtImpl const &) = delete;
    PreparedStmtImpl &operator=(PreparedStmtImpl &&) noexcept = default;

    /// @brief Executes the prepared statement.
    ///
    /// @return \c return_code for the statement execution.
    virtual ReturnCode execute() = 0;

    /// @brief Binds memory to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    virtual void bind_blob(int32_t const index, std::span<std::byte const> const &value) = 0;

    /// @brief Binds a boolean value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    virtual void bind_bool(int32_t const index, bool const value) = 0;

    /// @brief Binds a date value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    virtual void bind_date(int32_t const index, std::string_view date) = 0;

    /// @brief Binds a double value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    virtual void bind_double(int32_t const index, double const value) = 0;

    /// @brief Binds an in32_t value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    virtual void bind_int32(int32_t const index, int32_t const value) = 0;

    /// @brief Binds an in64_t value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    virtual void bind_int64(int32_t const index, int64_t const value) = 0;

    /// @brief Sets a prepared statement field to NULL.
    ///
    /// @param index Index of the column to bind \c value to.
    virtual void bind_null(int32_t const index) = 0;

    /// @brief Binds a string value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    virtual void bind_text(int32_t const index, std::string_view text) = 0;

    /// @brief Binds a UTF-16 string value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    virtual void bind_text16(int32_t const index, std::u16string_view text) = 0;

    /// @brief Binds a UUID value to a prepared statement field.
    ///
    /// @param index Index of the column to bind \c value to.
    /// @param value Data to bind to the prepared statement field.
    virtual void bind_uuid(int32_t const index, std::span<std::byte const> const &value) = 0;

    /// @brief Returns binary data from a prepared statement result.
    ///
    /// @return Binary data from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    virtual std::vector<std::byte> get_blob(int32_t const index) = 0;

    /// @brief Returns a \c bool value from a prepared statement result.
    ///
    /// @return \c bool value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    virtual bool get_bool(int32_t const index) = 0;

    /// @brief Returns a date value from a prepared statement result.
    ///
    /// @return Date value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    virtual std::string get_date(int32_t const index) = 0;

    /// @brief Returns a \c double value from a prepared statement result.
    ///
    /// @return \c double value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    virtual double get_double(int32_t const index) = 0;

    /// @brief Returns an \c int32_t value from a prepared statement result.
    ///
    /// @return \c int32_t value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    virtual int32_t get_int32(int32_t const index) = 0;

    /// @brief Returns an \c int64_t value from a prepared statement result.
    ///
    /// @return \c int64_t value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    virtual int64_t get_int64(int32_t const index) = 0;

    /// @brief Returns a text value from a prepared statement result.
    ///
    /// @return Text value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    virtual std::string get_text(int32_t const index) = 0;

    /// @brief Returns a UTF-16 text value from a prepared statement result.
    ///
    /// @return UTF-16 text value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    virtual std::u16string get_text16(int32_t const index) = 0;

    /// @brief Returns a UUID value from a prepared statement result.
    ///
    /// @return UUID value from the prepared statement result.
    ///
    /// @param index Index of the column to return the data for.
    virtual std::array<uint8_t, 16> get_uuid(int32_t const index) = 0;
};

} // namespace dbpool

#endif
