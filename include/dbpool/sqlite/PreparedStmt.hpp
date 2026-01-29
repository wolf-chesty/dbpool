#ifndef DBPOOL_SQLITE_PREPARED_STATEMENT_HPP
#define DBPOOL_SQLITE_PREPARED_STATEMENT_HPP

#include "dbpool/PreparedStmt.hpp"

#include <span>
#include <sqlite3.h>

namespace dbpool::sqlite {

class PreparedStmt : public dbpool::PreparedStmt {
public:
    PreparedStmt(PreparedStmt const &) = delete;
    PreparedStmt(PreparedStmt &&right) noexcept;
    explicit PreparedStmt(std::shared_ptr<dbpool::Connection> conn, sqlite3 *db, sqlite3_stmt *stmt) noexcept;
    ~PreparedStmt() override;

    PreparedStmt &operator=(PreparedStmt const &) = delete;
    PreparedStmt &operator=(PreparedStmt &&right) noexcept;

    return_code execute() override;

    void bind_blob(int32_t const index, std::span<std::byte const> const &value) override;
    void bind_bool(int32_t const index, bool const value) override;
    void bind_date(int32_t const index, std::string_view value) override;
    void bind_double(int32_t const index, double const value) override;
    void bind_int32(int32_t const index, int32_t const value) override;
    void bind_int64(int32_t const index, int64_t const value) override;
    void bind_null(int32_t const index) override;
    void bind_text(int32_t const index, std::string_view value) override;
    void bind_text16(int32_t const index, std::u16string_view value) override;
    void bind_uuid(int32_t const index, std::span<std::byte const> const &value) override;

    std::vector<std::byte> get_blob(int32_t index) override;
    bool get_bool(int32_t const index) override;
    std::string get_date(int32_t const index) override;
    double get_double(int32_t const index) override;
    int32_t get_int32(int32_t const index) override;
    int64_t get_int64(int32_t const index) override;
    std::string get_text(int32_t const index) override;
    std::u16string get_text16(int32_t const index) override;
    std::array<uint8_t, 16> get_uuid(int32_t const index) override;

    static return_code to_error_code(int code);

private:
    sqlite3 *m_db{};
    sqlite3_stmt *m_stmt{};
};

} // namespace dbpool::sqlite

#endif
