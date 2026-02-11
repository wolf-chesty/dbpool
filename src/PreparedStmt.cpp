// Copyright (c) 2026 Christopher L Walker
// SPDX-License-Identifier: MIT

#include "dbpool/PreparedStmt.hpp"

#include "dbpool/impl/PreparedStmtImpl.hpp"
#include "dbpool/util/PooledConnection.hpp"
#include <cassert>

using namespace dbpool;

PreparedStmt::PreparedStmt(std::shared_ptr<PooledConnection> pooled_conn,
                           std::unique_ptr<PreparedStmtImpl> impl) noexcept
    : pooled_conn_(std::move(pooled_conn))
    , impl_(std::move(impl))
{
    assert(pooled_conn_);
    assert(impl_);
}

PreparedStmt::~PreparedStmt()
{
    // Make sure members are destructed in the correct order. connection_ is a dependency of impl_ so make sure impl_
    // is destructed before connection_. We could rely on member ordering here but that would lead to fragile code and
    // things will break if someone reorders the members of this object.
    impl_.reset();
    pooled_conn_.reset();
}

PreparedStmt::ReturnCode PreparedStmt::execute()
{
    assert(impl_);
    return impl_->execute();
}

void PreparedStmt::bind_blob(int32_t const index, std::span<std::byte const> const &value)
{
    assert(impl_);
    impl_->bind_blob(index, value);
}

void PreparedStmt::bind_bool(int32_t const index, bool const value)
{
    assert(impl_);
    impl_->bind_bool(index, value);
}

void PreparedStmt::bind_date(int32_t const index, std::string_view value)
{
    assert(impl_);
    impl_->bind_date(index, value);
}

void PreparedStmt::bind_double(int32_t const index, double const value)
{
    assert(impl_);
    impl_->bind_double(index, value);
}

void PreparedStmt::bind_int32(int32_t const index, int32_t const value)
{
    assert(impl_);
    impl_->bind_int32(index, value);
}

void PreparedStmt::bind_int64(int32_t const index, int64_t const value)
{
    assert(impl_);
    impl_->bind_int64(index, value);
}

void PreparedStmt::bind_null(int32_t const index)
{
    assert(impl_);
    impl_->bind_null(index);
}

void PreparedStmt::bind_text(int32_t const index, std::string_view value)
{
    assert(impl_);
    impl_->bind_text(index, value);
}

void PreparedStmt::bind_text16(int32_t const index, std::u16string_view value)
{
    assert(impl_);
    impl_->bind_text16(index, value);
}

void PreparedStmt::bind_uuid(int32_t const index, std::span<std::byte const> const &value)
{
    assert(impl_);
    impl_->bind_uuid(index, value);
}

std::vector<std::byte> PreparedStmt::get_blob(int32_t const index)
{
    assert(impl_);
    return impl_->get_blob(index);
}

bool PreparedStmt::get_bool(int32_t const index)
{
    assert(impl_);
    return impl_->get_bool(index);
}

std::string PreparedStmt::get_date(int32_t const index)
{
    assert(impl_);
    return impl_->get_date(index);
}

double PreparedStmt::get_double(int32_t const index)
{
    assert(impl_);
    return impl_->get_double(index);
}

int32_t PreparedStmt::get_int32(int32_t const index)
{
    assert(impl_);
    return impl_->get_int32(index);
}

int64_t PreparedStmt::get_int64(int32_t const index)
{
    assert(impl_);
    return impl_->get_int64(index);
}

std::string PreparedStmt::get_text(int32_t const index)
{
    assert(impl_);
    return impl_->get_text(index);
}

std::u16string PreparedStmt::get_text16(int32_t const index)
{
    assert(impl_);
    return impl_->get_text16(index);
}

std::array<uint8_t, 16> PreparedStmt::get_uuid(int32_t const index)
{
    assert(impl_);
    return impl_->get_uuid(index);
}
