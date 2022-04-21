#pragma once

#include <libs/state/include/task.h>
#include <libs/state/include/vm.h>
#include <libs/postgres/include/pg_pool.h>

namespace vm_scheduler::testing {

TaskParameters getOneJobTaskParameters();
TaskParameters getThreeJobTaskParameters();
VmId insertVm(pg::PgPool& pool, const VmStatus status);

} // namespace vm_scheduler::testing
