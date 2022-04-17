#pragma once

#include <pqxx/pqxx>

#include <memory>

namespace vm_scheduler::pg {

class PgPool;

class ConnectionHandle {
public:
    ConnectionHandle(PgPool& pool, std::unique_ptr<pqxx::lazyconnection>&& connection);
    ConnectionHandle(ConnectionHandle&& other);

    ~ConnectionHandle();

    pqxx::lazyconnection& get();

private:
    PgPool& pool_;
    std::unique_ptr<pqxx::lazyconnection> connection_;
};

} // namespace vm_scheduler::pg
