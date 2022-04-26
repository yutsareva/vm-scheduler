#include <stdlib.h>

namespace vm_scheduler::testing {

void setupEnv()
{
    setenv("PG_USER", "postgres", true);
    setenv("PG_PASSWORD", "postgres", true);
    setenv("PG_DB_NAME", "postgres", true);
    setenv("PG_HOSTS", "host.docker.internal", true);
    setenv("PG_POOL_SIZE", "10", true);
}

}
