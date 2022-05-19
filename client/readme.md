## How to run
```bash
$ brew install protoc-gen-go
vm-scheduler/proto $ protoc --go_out=../client --go_opt=paths=source_relative \
  --go-grpc_out=../client --go-grpc_opt=paths=source_relative **/*.proto
vm-scheduler/client$ go mod tidy
// set AWS_ACCESS_KEY_ID, AWS_SECRET_ACCESS_KEY env vars
vm-scheduler/client$ VMS_AGENT_VM_ID=1 VMS_AGENT_SERVER_ADDRESS=localhost:50001 VMS_AGENT_AWS_ACCESS_KEY_ID=$AWS_ACCESS_KEY_ID VMS_AGENT_AWS_SECRET_ACCESS_KEY=$AWS_SECRET_ACCESS_KEY VMS_AGENT_AWS_REGION=us-east-2 VMS_AGENT_AWS_BUCKET=vms-results go run main.go
```


### Environment variables

| name                            | value                                     | required  | example                   |
| ------------------------------- | ----------------------------------------- | --------- | ------------------------- |
| VMS_AGENT_VM_ID                 | VM ID                                     | y         | 1                         |
| VMS_AGENT_SERVER_ADDRESS        | VM Scheduler server address               | y         | localhost:50001           |
| VMS_AGENT_AWS_ACCESS_KEY_ID     | AWS access key id, used for s3 connection | y         | AKI***A6Z                 |
| VMS_AGENT_AWS_SECRET_ACCESS_KEY | AWS secret access key, used for s3 conn   | y         | L33***9eY                 |
| VMS_AGENT_AWS_REGION            | AWS region, used for s3 conn              | y         | us-east-2                 |
| VMS_AGENT_AWS_BUCKET            | AWS s3 bucket                             | y         | bucket-name               |
