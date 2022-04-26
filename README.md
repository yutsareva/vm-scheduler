# Virtual machine scheduler (VMS)

Classes diagram:

![alternative text](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/yutsareva/vm-scheduler/main/docs/uml-diagrams/classes.puml)

Sequence diagram:

![alternative text](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/yutsareva/vm-scheduler/main/docs/uml-diagrams/workflow.puml)

Job state diagram:

![alternative text](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/yutsareva/vm-scheduler/main/docs/uml-diagrams/job_state.puml)

VM state diagram:

![alternative text](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/yutsareva/vm-scheduler/main/docs/uml-diagrams/vm_state.puml)

DB schema:

![alternative text](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/yutsareva/vm-scheduler/main/docs/uml-diagrams/db_schema.puml)

<!---
## Scenario (TBD refine)

- Initialize VMS with AWS/Azure credentials to allocate VMs from his Amazon/Azure accounts.
- Specify restrictions on VM / cpu allocated count.
- Register several `tasks` -- docker images to launch.
- Specify VM-spaces: sets of VMs are destined to launch specific tasks.
- Specify strategies to launch VMs (use on-demand/spot instances, always keep some idle VMs to reduce
  VM allocation and initialization time etc)
- `Tasks` are provided by client docker images that are parametrazed with some input data. `Tasks` can generate some output data
  (logs / task result) that should be saved to some persistent storage for some finite time.

## Architecture
<img src="./docs/scheduler-schema.drawio.svg?raw=true" width="800">

### Sequence diagram
<img src="./docs/vm_scheduler_seq_v3.drawio.svg?raw=true" width="800">

## Entities (TBD refine)

### Tasks & VMs Storage
- Stores status of tasks
- Stores data about allocated and terminated VMs
- Assumed to have two implementations:
  - Postgres with Optimistic Concurrent Locking model
  - Postgres for storing finished tasks and Zookeeper for storing current plan
- Interfaces: https://github.com/yutsareva/vm-scheduler/tree/main/libs/task_storage

### Output/input storage
- Stores tasks' inputs/outputs

### Core Scheduler
- Gets info about all tasks for the client (required cpu, memory, job count), allocated VMs
  and finds an appropriate VM or decides to allocate a new one to launch the task.
- VM Assigner is a part of core scheduler
  - Implements an algorithm to choose VMs for tasks
- Interfaces: https://github.com/yutsareva/vm-scheduler/tree/main/libs/scheduler

### Allocator
- Allocates new VMs
- Terminates idle VMs
- Cloud Client implements communication with cloud providers
- Interfaces: https://github.com/yutsareva/vm-scheduler/tree/main/libs/allocator

### Agent
- Launches on allocated instaces
- Pulls assigned tasks
- Launches tasks
- Pushes tasks' states to scheduler

### Failure Detector
- Monitors state of launched tasks, allocated instances
- Finds failed tasks / failed instances

### DockerHub
- Stores docker images

### Task Registry
- Server base

## API (TBD refine)
### Server API for users
- Register client profile (provides credentials to allocate instances from their cloud account)
- Register task types (docker images)
- Register VM-spaces
- Launch task
- Get status of task
- Get state/result of task
- Get stats
- Get profile info/stats
### Server API for agents running on allocated instances (TBD)


## Code (-design/-style) rules
- Clean architecture - any usage of db/storage/another service should be an interface
- No exceptions are expected to be catched from library calls, Result<error, value> should be used
-->
