# Virtual machine scheduler as a service (VMSaaS)

## Use cases

- Client registers his own profile in VMSaaS, provides some credentials to allocate VMs from his Amazon/Azure accounts.
- Client can specify restrictions on VM / cpu allocated count.
- Client can register several `tasks` -- binaries they want to launch.
- Client can specify VM-spaces: sets of VMs are destined to launch specific tasks.
- Client can specify strategies to launch VMs (use on-demand/spot instances, always keep some idle VMs to reduce
  VM allocation and initialization time etc)

- `Tasks` are provided by client binaries initialized with some data. `Tasks` can generate some output data
  (logs / task result) that should be saved to some persistent storage for some finite time.


## Entities

### Task queue
- Stores status of tasks

### Allocated VM storage
- Stores data about allocated and deallocated VMs

### Binary storage
- Stores task binaries

### Output storage
- Stores task outputs

### Decision maker (VM selector)
- Gets info about all tasks for the client (deadline, cpu, memory), allocated VMs and finds an appropriate VM 
  or decides to allocate a new one to launch the task

### Allocator
- Allocates new VMs
- Deallocated idle VMs

### Allocated instances storage
- Stores info about allocated instances

### Task Launcher
- Launches tasks

### Fail detector
- Monitors state of launched tasks, allocated instances. Finds failed tasks / failed instances


## API

- Register client profile (provides credentials to allocate instances from their cloud account)
- Register task types (binaries)
- Register VM-spaces
- Launch task
- Get status of task
- Get result of task
- Get stats
- Get profile info/stats


## Code (-design/-style) rules
- No exceptions are expected to catch, Result<error, value> should be used otherwise
- Strong types should be used for primitive types
- Clean architecture - any usage of db/storage/another service should be an interface
- Use namespaces


