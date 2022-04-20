package poller

import (
	"context"
	"google.golang.org/grpc/grpclog"
	agent_api "scheduler/registry"
	pb_api "scheduler/services"
	pb "scheduler/structures"
)

func getAssignments(vmId uint64, client pb_api.AgentApiSchedulerClient) *JobAssignments {
	protoVmId := &pb.VmId{
		Value: &vmId,
	}
	response, err := client.GetAssignedJobs(context.Background(), protoVmId)
	if err != nil {
		grpclog.Fatalf("fail to dial: %v", err)
		return &JobAssignments{}
	}
	var assignedJobIds []agent_api.JobId
	var cancelledJobIds []agent_api.JobId
	for _, job := range response.Jobs {
		if *job.Status == pb.JobStatus_JOB_CANCELLED {
			cancelledJobIds = append(cancelledJobIds, agent_api.JobId(*job.Id.Value))
		} else {
			assignedJobIds = append(assignedJobIds, agent_api.JobId(*job.Id.Value))
		}
	}
	return &JobAssignments{
		assignedJobIds,
		cancelledJobIds,
	}
}

func getAssignedJobInfo(client pb_api.AgentApiSchedulerClient, vmId uint64, jobId uint64) *agent_api.JobInfo {
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
	return &agent_api.JobInfo{
		ImageVersion: *response.ImageVersion,
		Limits: agent_api.JobLimits{
			Cpu: *response.JobLimits.CpuCores,
			Ram: *response.JobLimits.MemoryMb,
		},
		TaskSettings: *response.TaskSettings,
		JobOptions:   *response.JobOptions,
	}
}
