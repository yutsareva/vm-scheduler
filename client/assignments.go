package agent_api

import (
	"context"
	"google.golang.org/grpc/grpclog"
	pb_api "scheduler/services"
	pb "scheduler/structures"
	"time"
)

type JobAssignments struct {
	assignedJobIds []JobId
	cancelledJobIds []JobId
}

func getAssignments(vmId uint64, client pb_api.AgentApiSchedulerClient) *JobAssignments {
	protoVmId := &pb.VmId{
		Value: &vmId,
	}
	response, err := client.GetAssignedJobs(context.Background(), protoVmId)
	if err != nil {
		grpclog.Fatalf("fail to dial: %v", err)
		return &JobAssignments{}
	}
	var assignedJobIds []JobId
	var cancelledJobIds []JobId
	for _, job := range response.Jobs {
		if *job.Status == pb.JobStatus_JOB_CANCELLED {
			cancelledJobIds = append(cancelledJobIds, JobId(*job.Id.Value))
		} else {
			assignedJobIds = append(assignedJobIds, JobId(*job.Id.Value))
		}
	}
	return &JobAssignments{
		assignedJobIds,
		cancelledJobIds,
	}
}

func getAssignedJobInfo(client pb_api.AgentApiSchedulerClient, vmId uint64, jobId uint64) *JobInfo {
	protoLaunchRequest := &pb.LaunchRequest{
		VmId: &pb.VmId{
			Value: &vmId,
		},
		JobId: &pb.JobId{
			Value: &jobId,
		},
	}
	response, err := client.GetJobToLaunch(context.Background(), protoLaunchRequest)
	if err != nil {
		grpclog.Fatalf("fail to dial: %v", err)
		return nil
	}
	return &JobInfo{
		*response.ImageVersion,
		JobLimits{
			*response.JobLimits.CpuCores,
			*response.JobLimits.MemoryMb,
		},
		*response.TaskSettings,
		*response.JobOptions,
	}
}

func pollSchedulerForAssignments(
		client pb_api.AgentApiSchedulerClient, config *Config, state *State) chan struct{} {
	ticker := time.NewTicker(config.pollInterval)
	quit := make(chan struct{})
	go func() {
		for {
			select {
			case <- ticker.C:
				assignments := getAssignments(config.vmId, client)
				if len(assignments.cancelledJobIds) > 0 {
					state.addCancelledJobs(assignments.cancelledJobIds)
				}
				if len(assignments.assignedJobIds) > 0 {
					state.addAssignedJobs(assignments.assignedJobIds)
				}
				assignedJobs := state.getJobsForInfo()
				for _, jobId := range assignedJobs {
					maybeJobInfo := getAssignedJobInfo(client, config.vmId, uint64(jobId))
					if maybeJobInfo != nil {
						state.updateJobInfos(jobId, maybeJobInfo)
					}
				}
			case <- quit:
				ticker.Stop()
				return
			}
		}
	}()

	return quit
}



