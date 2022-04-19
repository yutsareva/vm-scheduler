package runner

import (
	"context"
	"fmt"
	"github.com/docker/docker/api/types/container"
	"github.com/docker/docker/client"
	"log"
	agent_api "scheduler"
	"scheduler/docker_utils"
	"scheduler/s3_utils"
	pb_api "scheduler/services"
	pb "scheduler/structures"
	"time"
)


func handleContainerCompletion(
		ctx context.Context, cli *client.Client, containerId *string, client pb_api.AgentApiSchedulerClient,
		jobId uint64, vmId uint64, resultFileName *string, s3Manager *s3_utils.S3Manager) {
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
				log.Printf("Failed to update job state for job %d: %v", jobId, err)
			}
			return
		}
	case <-statusCh:
		status := pb.JobStatus_JOB_COMPLETED

		key := resultFileName // TODO: key
		resultUrl, err := s3Manager.UploadFileToS3(resultFileName, key)

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
		_, err = client.UpdateJobState(ctx, protoExecutionJobState)
		if err != nil {
			log.Printf("Failed to update job state for job %d: %v", jobId, err)
		}
		return
	}
}

func runJobContainer(jobId agent_api.JobId, jobInfo *agent_api.JobInfo) (*string, error) {
	cli, err := client.NewEnvClient()
	if err != nil {
		return nil, err
	}

	jobName := fmt.Sprintf("job-%d", jobId)

	ctx := context.Background()
	err = docker_utils.PullImage(ctx, cli, &jobInfo.ImageVersion)
	if err != nil {
		return nil, err
	}

	containerId, err := docker_utils.CreateContainer(ctx, cli, &jobName, &jobInfo.ImageVersion)
	if err != nil {
		return nil, err
	}

	err = docker_utils.StartContainer(ctx, cli, containerId)
	if err != nil {
		return nil, err
	}

	//go handleContainerCompletion(ctx, cli, containerId)

	return containerId, nil
}

func runJobs(
	client pb_api.AgentApiSchedulerClient, config *agent_api.Config, state *agent_api.State, s3Manager *s3_utils.S3Manager) chan struct{} {
	ticker := time.NewTicker(config.JobLaunchInterval)
	quit := make(chan struct{})
	go func() {
		for {
			select {
			case <- ticker.C:
				jobId, jobInfo := state.GetReadyToRunJob()
				if jobId == nil {
					continue
				}
				containerId, err := runJobContainer(*jobId, jobInfo)
				if err != nil {
					state.ReturnFailedToLaunchJob(*jobId)
				}
				//cli, err := client.NewEnvClient()
				//if err != nil {
				//	return nil, err
				//}
				//
				//jobName := fmt.Sprintf("job-%d", jobId)
				//
				ctx := context.Background()
				handleContainerCompletion(ctx, cli, containerId, client, uint64(*jobId), config.VmId, s3Manager)
			case <- quit:
				ticker.Stop()
				return
			}
		}
	}()

	return quit
}
