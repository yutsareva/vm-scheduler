package agent_api

import (
    "log"
    "scheduler/poller"
    registry2 "scheduler/registry"
)

func main() {
        log.SetFlags(log.LstdFlags)

        registry := registry2.createRegistry()

        pollChan := poller.PollSchedulerForAssignments(registry)

        // ...

        close(pollChan)

        registry.close()
}
