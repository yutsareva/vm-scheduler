package runner

import (
	"context"
	"log"
	pb_api "scheduler/services"
	pb "scheduler/structures"
)

func updateJobState(
	ctx context.Context, status pb.JobStatus, resultUrl *string, jobId uint64, vmId uint64, client pb_api.AgentApiSchedulerClient) {
	protoExecutionJobState := &pb.ExecutionJobState{
		JobId: &pb.JobId{
			Value: &jobId,
		},
		VmId: &pb.VmId{
			Value: &vmId,
		},
		JobResult: &pb.JobExecutionResult{
			Status: &status,
			ResultUrl: resultUrl,
		},
	}
	_, err := client.UpdateJobState(ctx, protoExecutionJobState)
	if err != nil {
		log.Printf("Failed to update job state for job %d: %v", jobId, err)
	}
	return
}