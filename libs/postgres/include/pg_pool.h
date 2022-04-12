#pragma once

#include "libs/postgres/impl/txn.h"
#include "libs/postgres/impl/connection.h"

#include <pqxx/pqxx>

#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>


namespace vm_scheduler {

class PgPool {
public:
    PgPool(const size_t poolSize);

    TransactionHandle readOnlyTransaction()
    {
        ConnectionHandle connectionHandle(*this, connection());

        auto txn = std::make_unique<pqxx::read_transaction>(connectionHandle.get());
        return {std::move(connectionHandle), std::move(txn)};
    }
    TransactionHandle writableTransaction()
    {
        ConnectionHandle connectionHandle(*this, connection());

        auto txn = std::make_unique<pqxx::transaction<>>(connectionHandle.get());
        return {std::move(connectionHandle), std::move(txn)};
    }

    friend ConnectionHandle;

private:
    std::unique_ptr<pqxx::lazyconnection> connection()
    {
        std::unique_lock<std::mutex> lock(mutex_);

        while (connectionPool_.empty()) {
            poolNotEmpty_.wait(lock);
        }

        auto conn = std::move(connectionPool_.front());
        connectionPool_.pop();

        return conn;
    }
    void freeConnection(std::unique_ptr<pqxx::lazyconnection>) { }

private:
    std::mutex mutex_;
    std::condition_variable poolNotEmpty_;
    std::queue<std::unique_ptr<pqxx::lazyconnection>> connectionPool_;
};

} // namespace vm_scheduler
