#include "libs/postgres/include/pg_pool.h"

#include <libs/common/include/log.h>

namespace vm_scheduler::pg {

PgPool::PgPool(const size_t poolSize, const std::string& connectionString)
{
    for (auto i = 0; i < poolSize; i++) {
        connectionPool_.emplace(
            std::make_unique<pqxx::lazyconnection>(connectionString));
    }
}

TransactionHandle PgPool::readOnlyTransaction()
{
    ConnectionHandle connectionHandle(*this, connection());

    auto txn = std::make_unique<pqxx::read_transaction>(connectionHandle.get());
    return {std::move(connectionHandle), std::move(txn)};
}

TransactionHandle PgPool::writableTransaction()
{
    ConnectionHandle connectionHandle(*this, connection());

    auto txn = std::make_unique<pqxx::transaction<>>(connectionHandle.get());
    return {std::move(connectionHandle), std::move(txn)};
}

std::unique_ptr<pqxx::lazyconnection> PgPool::connection()
{
    std::unique_lock<std::mutex> lock(mutex_);

    while (connectionPool_.empty()) {
        poolNotEmpty_.wait(lock);
    }
    DEBUG() << "PG pool size: " << connectionPool_.size();

    auto conn = std::move(connectionPool_.front());
    connectionPool_.pop();

    return conn;
}

void PgPool::freeConnection(std::unique_ptr<pqxx::lazyconnection>&& connection)
{
    {
        std::unique_lock<std::mutex> lock(mutex_);
        connectionPool_.push(std::move(connection));
    }

    poolNotEmpty_.notify_one();
}

} // namespace vm_scheduler::pg
