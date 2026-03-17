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

PreparedStmt::ReturnCode PreparedStmt::reset()
{
    assert(impl_);
    return impl_->reset();
}

bool PreparedStmt::isNull(int32_t const index)
{
    assert(impl_);
    return impl_->isNull(index);
}

void PreparedStmt::bindBlob(int32_t const index, std::span<std::byte const> const &value)
{
    assert(impl_);
    impl_->bindBlob(index, value);
}

void PreparedStmt::bindBool(int32_t const index, bool const value)
{
    assert(impl_);
    impl_->bindBool(index, value);
}

void PreparedStmt::bindDate(int32_t const index, std::string_view value)
{
    assert(impl_);
    impl_->bindDate(index, value);
}

void PreparedStmt::bindDouble(int32_t const index, double const value)
{
    assert(impl_);
    impl_->bindDouble(index, value);
}

void PreparedStmt::bindInt32(int32_t const index, int32_t const value)
{
    assert(impl_);
    impl_->bindInt32(index, value);
}

void PreparedStmt::bindInt64(int32_t const index, int64_t const value)
{
    assert(impl_);
    impl_->bindInt64(index, value);
}

void PreparedStmt::bindNull(int32_t const index)
{
    assert(impl_);
    impl_->bindNull(index);
}

void PreparedStmt::bindText(int32_t const index, std::string_view value)
{
    assert(impl_);
    impl_->bindText(index, value);
}

void PreparedStmt::bindText16(int32_t const index, std::u16string_view value)
{
    assert(impl_);
    impl_->bindText16(index, value);
}

void PreparedStmt::bindUuid(int32_t const index, std::span<std::byte const> const &value)
{
    assert(impl_);
    impl_->bindUuid(index, value);
}

std::vector<std::byte> PreparedStmt::getBlob(int32_t const index)
{
    assert(impl_);
    return impl_->getBlob(index);
}

bool PreparedStmt::getBool(int32_t const index)
{
    assert(impl_);
    return impl_->getBool(index);
}

std::string PreparedStmt::getDate(int32_t const index)
{
    assert(impl_);
    return impl_->getDate(index);
}

double PreparedStmt::getDouble(int32_t const index)
{
    assert(impl_);
    return impl_->getDouble(index);
}

int32_t PreparedStmt::getInt32(int32_t const index)
{
    assert(impl_);
    return impl_->getInt32(index);
}

int64_t PreparedStmt::getInt64(int32_t const index)
{
    assert(impl_);
    return impl_->getInt64(index);
}

std::string PreparedStmt::getText(int32_t const index)
{
    assert(impl_);
    return impl_->getText(index);
}

std::u16string PreparedStmt::getText16(int32_t const index)
{
    assert(impl_);
    return impl_->getText16(index);
}

std::array<std::byte, 16> PreparedStmt::getUuid(int32_t const index)
{
    assert(impl_);
    return impl_->getUuid(index);
}