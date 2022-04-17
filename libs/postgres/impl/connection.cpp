#include "libs/postgres/impl/connection.h"
#include "libs/postgres/include/pg_pool.h"
#include "libs/common/include/log.h"

namespace vm_scheduler::pg {

ConnectionHandle::ConnectionHandle(PgPool& pool, std::unique_ptr<pqxx::lazyconnection>&& connection)
    : pool_(pool), connection_(std::move(connection))
{
}

ConnectionHandle::ConnectionHandle(ConnectionHandle&& other)
    : pool_(other.pool_)
    , connection_(std::move(other.connection_))
{
    other.connection_ =  nullptr;
}

ConnectionHandle::~ConnectionHandle()
{
    if (connection_) {
        pool_.freeConnection(std::move(connection_));
    }
}

pqxx::lazyconnection& ConnectionHandle::get()
{
    return *connection_;
}

} // namespace vm_scheduler::pg
