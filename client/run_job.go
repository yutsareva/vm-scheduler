package agent_api

import (
	"context"
	"fmt"
	"github.com/docker/docker/api/types/container"
	"github.com/docker/docker/client"
	pb_api "scheduler/services"
	pb "scheduler/structures"
	"time"
)

func saveResultToS3() string {
	// TODO
	return ""
}

func handleContainerCompletion(
		ctx context.Context, cli *client.Client, containerId *string, client pb_api.AgentApiSchedulerClient,
		jobId uint64, vmId uint64) {
	statusCh, errCh := cli.ContainerWait(ctx, *containerId, container.WaitConditionNotRunning)
	select {
	case err := <-errCh:
		if err != nil {
			status := pb.JobStatus_JOB_FAILED
			protoExecutionJobState := &pb.ExecutionJobState{
				JobId: &pb.JobId{
					Value: &jobId,
				},
				VmId: &pb.VmId{
					Value: &vmId,
				},
				JobResult: &pb.JobExecutionResult{
					Status: &status,
				},
			}
			_, err := client.UpdateJobState(ctx, protoExecutionJobState)
			if err != nil {
				// TODO
			}
			return
		}
	case <-statusCh:
		status := pb.JobStatus_JOB_COMPLETED

		resultUrl := saveResultToS3()

		protoExecutionJobState := &pb.ExecutionJobState{
			JobId: &pb.JobId{
				Value: &jobId,
			},
			VmId: &pb.VmId{
				Value: &vmId,
			},
			JobResult: &pb.JobExecutionResult{
				Status: &status,
				ResultUrl: &resultUrl,
			},
		}
		_, err := client.UpdateJobState(ctx, protoExecutionJobState)
		if err != nil {
			// TODO
		}
		return
	}
}

func runJobs(
		client pb_api.AgentApiSchedulerClient, config *Config, state *State) chan struct{} {
	ticker := time.NewTicker(config.jobLaunchInterval)
	quit := make(chan struct{})
	go func() {
		for {
			select {
			case <- ticker.C:
				jobId, jobInfo := state.getReadyToRunJob()
				if jobId == nil {
					continue
				}
				containerId, err := runJobContainer(*jobId, jobInfo)
				if err != nil {
					state.returnFailedToLaunchJob(*jobId)
				}
				cli, err := client.NewEnvClient()
				//if err != nil {
				//	return nil, err
				//}
				//
				//jobName := fmt.Sprintf("job-%d", jobId)
				//
				ctx := context.Background()
				handleContainerCompletion(ctx, cli, containerId, client, uint64(*jobId), config.vmId)
			case <- quit:
				ticker.Stop()
				return
			}
		}
	}()

	return quit
}
