#include "libs/postgres/impl/connection.h"
#include "libs/postgres/include/pg_pool.h"


namespace vm_scheduler {

ConnectionHandle::ConnectionHandle(
        PgPool& pool,
        std::unique_ptr<pqxx::lazyconnection>&& connection)
    : pool_(pool),
    connection_(std::move(connection))
{
}

ConnectionHandle::~ConnectionHandle()
{
    pool_.freeConnection(std::move(connection_));
}

pqxx::lazyconnection& ConnectionHandle::get()
{
    return *connection_;
}

} // namespace vm_scheduler
