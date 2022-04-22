#include "libs/db/impl/test_utils.h"

#include <libs/common/include/stringify.h>

#include <libs/postgres/include/helpers.h>

#include <chrono>

namespace vm_scheduler::testing {

using namespace std::chrono_literals;

TaskParameters getOneJobTaskParameters()
{
    return TaskParameters{
        .requiredCapacity =
            {
                .cpu = 1_cores,
                .ram = 1024_MB,
            },
        .estimation = 60s,
        .jobCount = 1,
        .settings = "{}",
        .imageVersion = "no bugs version :)",
        .clientId = "client id",
        .jobOptions = {"{\"options\": []}"},
    };
}

TaskParameters getThreeJobTaskParameters()
{
    return TaskParameters{
        .requiredCapacity =
            {
                .cpu = 1_cores,
                .ram = 1024_MB,
            },
        .estimation = 60s,
        .jobCount = 3,
        .settings = "{}",
        .imageVersion = "deprecated version",
        .clientId = "client id",
        .jobOptions =
            {"[\"options\", \"text\"]", "{\"a\": 42}", "{\"abc\": [1, 2, 42]}"},
    };
}

VmId insertVm(pg::PgPool& pool, const VmStatus status)
{
    auto writeTxn = pool.writableTransaction();
    const auto insertVmQuery = toString(
        "INSERT INTO scheduler.vms ",
        "(status, created, cpu, ram, cpu_idle, ram_idle, cloud_vm_id, cloud_vm_type, last_status_update) ",
        "VALUES('",
        toString(status),
        "', NOW(), 2, 2048, 1, 1024, 'cloud vm id', 'cloud vm type', NOW()) "
        "RETURNING id;");
    const auto vmResult = pg::execQuery(insertVmQuery, *writeTxn);
    writeTxn->commit();
    return vmResult[0].at("id").as<VmId>();
}

} // namespace vm_scheduler::testing
