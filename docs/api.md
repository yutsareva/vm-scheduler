## gRPC API provided by the VM Scheduler

### Public API

- Add task to the Scheduler:
```bash
grpcurl -d \
'{
  "limits": {
    "job_limits": {
      "memory_mb": 2,
      "cpu_cores": 1
    },
    "execution_s": 5
  },
  "settings": "{\"a\":\"b\"}",
  "job_count": 2,
  "image_version":"img version",
  "client_id":"yutsareva",
  "job_options":["{}","[]"]
}' \
-plaintext -import-path <path> -\
proto public_api.proto <VM Scheduler address> \
vm_scheduler.proto.PublicApiScheduler/addTask

{
  "taskId": {
    "value": "4"
  }
}
```
- Get job statuses for the task_id
```bash
grpcurl -d '{"value":1}' -plaintext -import-path <path> \
-proto scheduler.proto <VM Acheduler address> \
vm_scheduler.proto.PublicApiScheduler/getTaskResult

{
  "status": "TASK_QUEUED",
  "jobResults": [
    {
      "status": "JOB_QUEUED"
    },
    {
      "status": "JOB_QUEUED"
    }
  ]
}
```
- Get job statuses with results. Contains field "result_url" with URL to result in S3 if the job was successfully completed.
```bash
grpcurl -d '{"value":1}' -plaintext -import-path <path> \
-proto scheduler.proto <VM Scheduler address> \
vm_scheduler.proto.PublicApiScheduler/getTaskState

{
  "status": "TASK_QUEUED",
  "jobStates": [
    {
      "status": "JOB_RUNNING"
    },
    {
      "status": "JOB_COMPLETED",
      "result_utl": "url to result on s3"
    }
  ]
}
```

### API for communication with the agents launched on allocated virtual machines 

- Get assigned jobs
```bash
grpcurl -d '{"value": 18}' -plaintext -import-path <path> \
-proto agent_api.proto <VM Scheduler address> \
vm_scheduler.proto.AgentApiScheduler/getAssignedJobs

{
  "jobs": [
    {
      "id": {
        "value": "6"
      },
      "status": "JOB_QUEUED"
    }
  ]
}
```
- Get job data
```bash
grpcurl -d '{"vm_id": {"value": 18}, "job_id": {"value": 6}}' -plaintext -import-path <path> \
-proto agent_api.proto <VM Scheduler address> \
vm_scheduler.proto.AgentApiScheduler/getJobToLaunch

{
  "id": {
    "value": "6"
  },
  "imageVersion": "img version",
  "jobLimits": {
    "memoryMb": 2,
    "cpuCores": 1
  },
  "taskSettings": "{\"a\": \"b\"}",
  "jobOptions": "[]"
}
```
- Update job status
```bash
grpcurl -d \
'{
  "vm_id": {"value": 18},
  "job_id": {"value": 6},
  "job_result": {
    "status": "JOB_RUNNING"
  }
}' \
-plaintext -import-path <path> -proto agent_api.proto <VM Scheduler address> \
vm_scheduler.proto.AgentApiScheduler/updateJobState

{
}
```
