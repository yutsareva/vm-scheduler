#include "libs/postgres/impl/txn.h"


namespace vm_scheduler {

TransactionHandle::TransactionHandle(
        ConnectionHandle&& connectionHandle,
        std::unique_ptr<pqxx::transaction_base>&& txn)
    : connectionHandle_(std::move(connectionHandle))
    , txn_(std::move(txn))
{
}

pqxx::transaction_base* TransactionHandle::operator->() { return txn_.get(); }

} // namespace vm_scheduler
