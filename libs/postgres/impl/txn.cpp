#include "libs/postgres/impl/txn.h"

namespace vm_scheduler::pg {

TransactionHandle::TransactionHandle(ConnectionHandle&& connectionHandle, std::unique_ptr<pqxx::transaction_base>&& txn)
    : txn_(std::move(txn))
    , connectionHandle_(std::move(connectionHandle))
{ }

pqxx::transaction_base* TransactionHandle::operator->()
{
    return txn_.get();
}

pqxx::transaction_base& TransactionHandle::operator*()
{
    return *txn_.get();
}


} // namespace vm_scheduler::pg
