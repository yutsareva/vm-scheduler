package agent_api

import (
	"context"
	"google.golang.org/grpc/grpclog"
	pb_api "scheduler/services"
	pb "scheduler/structures"
	"time"
)

func getAssignments(vmId uint64, client pb_api.AgentApiSchedulerClient) []JobId {
	protoVmId := &pb.VmId{
		Value: &vmId,
	}
	response, err := client.GetAssignedJobs(context.Background(), protoVmId)
	if err != nil {
		grpclog.Fatalf("fail to dial: %v", err)
	}
	var jobIds []JobId
	for _, job := range response.Jobs {
		jobIds = append(jobIds, JobId(*job.Id.Value))
	}
	return jobIds
}

func pollSchedulerForAssignments(
		vmId uint64, client pb_api.AgentApiSchedulerClient, config *Config, state *State) chan struct{} {
	ticker := time.NewTicker(config.pollInterval)
	quit := make(chan struct{})
	go func() {
		for {
			select {
			case <- ticker.C:
				jobIds := getAssignments(vmId, client)
				if len(jobIds) > 0 {
					state.updateAssignments(jobIds)
				}
			case <- quit:
				ticker.Stop()
				return
			}
		}
	}()

	return quit
}



