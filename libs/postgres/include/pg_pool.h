#pragma once

#include "libs/postgres/impl/connection.h"
#include "libs/postgres/impl/txn.h"

#include <pqxx/pqxx>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

namespace vm_scheduler::pg {

class PgPool {
public:
    PgPool(const size_t poolSize);
    PgPool(PgPool&& pool)
        : mutex_()
        , poolNotEmpty_()
        , connectionPool_(std::move(pool.connectionPool_))
    {}

    TransactionHandle readOnlyTransaction();
    TransactionHandle writableTransaction();

    friend class ConnectionHandle;

private:
    std::unique_ptr<pqxx::lazyconnection> connection();
    void freeConnection(std::unique_ptr<pqxx::lazyconnection>&& connection);

private:
    std::mutex mutex_;
    std::condition_variable poolNotEmpty_;
    std::queue<std::unique_ptr<pqxx::lazyconnection>> connectionPool_;
};

} // namespace vm_scheduler::pg
