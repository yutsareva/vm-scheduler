# Virtual machine scheduler (VMS)

The scheduler runs tasks on a dynamic pool of resources.
The task consists of several jobs.
Every job is a computational task.

The task is defined by the docker image and parameters to run.
Each job also has its own parameters, which are passed to the container as arguments at startup, while the docker image is the same for all jobs of the task.
All jobs have the same resource requirements.

The scheduler runs tasks on virtual machines that it allocates in the AWS cloud provider.
An agent is launched on the allocated machines.
The agent runs jobs in docker containers and communicates with the scheduler (to receive assigned tasks, send statuses of running tasks, heartbeats).

## API

The scheduler provides gRPC API for users and agents.

Examples: [api.md](docs/api.md)

## Architecture

System architecture, scenario description and implementation details are described in [architecture.md](docs/architecture.md).
