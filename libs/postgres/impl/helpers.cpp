#include "libs/postgres/include/helpers.h"

#include <libs/common/include/env.h>
#include <libs/common/include/log.h>


namespace vm_scheduler::pg {

pqxx::result execQuery(const std::string& sql, pqxx::transaction_base& w)
{
//    INFO() << "SQL query execution: " << sql;
    try {
        return w.exec(sql);
    } catch (const std::exception& ex) {
        ERROR() << "SQL query execution failed: " << ex.what();
        w.abort();
        throw;
    }
}

namespace {

std::string getConnectionString(const std::optional<std::string>& connectionString)
{
    if (connectionString) {
        return *connectionString;
    }
    const auto user = getFromEnvOrThrow("PG_USER");
    const auto dbName = getFromEnvOrThrow("PG_DB_NAME");
    const auto hosts = getFromEnvOrThrow("PG_HOSTS");
    const auto password = getFromEnvOrThrow("PG_PASSWORD");
    const auto port = getFromEnvOrDefault("PG_PORT", 5432);

    return toString(
        "user=", user, " ",
        "dbname=", dbName, " ",
        "host=", hosts, " ",
        "password=", password, " ",
        "port=", port
    );
}

size_t getPoolSize(const std::optional<size_t>& poolSize)
{
    if (poolSize) {
        return *poolSize;
    }
    return std::atoi(getFromEnvOrThrow("PG_POOL_SIZE").c_str());
}

} // anonymous namespace

PgPool createPool(const std::optional<std::string>& connectionString, const std::optional<size_t>& poolSize)
{
    return PgPool(getPoolSize(poolSize), getConnectionString(connectionString));
}

} // namespace vm_scheduler::pg
