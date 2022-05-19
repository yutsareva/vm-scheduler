package runner

import (
	"context"
	"fmt"
	"github.com/docker/docker/api/types/container"
	"log"
	"scheduler/docker_utils"
	"scheduler/registry"
	pb "scheduler/structures"
	"time"
)

func handleContainerCompletion(
	ctx context.Context, registry *registry.Registry, containerId *string,
	jobId uint64, resultFileName *string) {
	log.Printf("Wait container %v to finish", containerId)
	statusCh, errCh := registry.DockerClient.ContainerWait(ctx, *containerId, container.WaitConditionNotRunning)
	log.Printf("Container %v finished", containerId)
	select {
	case err := <-errCh:
		log.Printf("Container %v finished with error", containerId)
		if err != nil {
			status := pb.JobStatus_JOB_FAILED
			updateJobState(ctx, status, nil, jobId, registry.Config.VmId, registry.Client)
		}
	case <-statusCh:
		status := pb.JobStatus_JOB_COMPLETED
		key := resultFileName // TODO: key
		resultUrl, err := registry.S3Manger.UploadFileToS3(resultFileName, key)
		if err != nil {
			log.Printf("Failed to upload result to s3 %d: %v", jobId, err)
			return
		}
		updateJobState(ctx, status, resultUrl, jobId, registry.Config.VmId, registry.Client)
	}
	registry.State.CompleteJob(jobId)
}

func runJobContainer(jobId registry.JobId, jobInfo *registry.JobInfo, registry *registry.Registry) error {
	jobName := fmt.Sprintf("job-%d", jobId)
	log.Printf("Launching job container %v", jobName)

	ctx := context.Background()
	err := docker_utils.PullImage(ctx, registry.DockerClient, &jobInfo.ImageVersion)
	if err != nil {
		return err
	}
	log.Printf("Pulled image %v", jobInfo.ImageVersion)

	containerId, err := docker_utils.CreateContainer(ctx, registry.DockerClient, &jobName, jobInfo)
	if err != nil {
		return err
	}
	log.Printf("Created container %v", containerId)

	err = docker_utils.StartContainer(ctx, registry.DockerClient, containerId)
	if err != nil {
		return err
	}
	log.Printf("Started container %v", containerId)

	resultFile := "/" + jobName + "/result.json"
	go handleContainerCompletion(ctx, registry, containerId, uint64(jobId), &resultFile)

	return nil
}

func RunJobs(
	registry *registry.Registry) chan struct{} {
	ticker := time.NewTicker(registry.Config.JobLaunchInterval)
	quit := make(chan struct{})
	go func() {
		for {
			select {
			case <-ticker.C:
				for {
					jobId, jobInfo := registry.State.GetReadyToRunJob()
					if jobId == nil {
						break
					}
					err := runJobContainer(*jobId, jobInfo, registry)
					if err != nil {
						log.Printf("Failed to run container: %v", err)
						registry.State.ReturnFailedToLaunchJob(*jobId)
					}
				}
			case <-quit:
				ticker.Stop()
				return
			}
		}
	}()

	return quit
}
