#pragma once

#include "libs/postgres/impl/connection.h"

#include <pqxx/pqxx>

#include <memory>

namespace vm_scheduler::pg {

class TransactionHandle {
public:
    TransactionHandle(ConnectionHandle&& connectionHandle, std::unique_ptr<pqxx::transaction_base>&& txn);

    pqxx::transaction_base* operator->();
    pqxx::transaction_base& operator*();

private:
    std::unique_ptr<pqxx::transaction_base> txn_;
    ConnectionHandle connectionHandle_;
};

} // namespace vm_scheduler::pg
