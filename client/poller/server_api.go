package poller

import (
	"context"
	"log"
	agent_api "scheduler/registry"
	pb_api "scheduler/services"
	pb "scheduler/structures"
)

func getAssignments(vmId uint64, client pb_api.AgentApiSchedulerClient) *JobAssignments {
	log.Print("Getting assignments...")
	protoVmId := &pb.VmId{
		Value: &vmId,
	}
	response, err := client.GetAssignedJobs(context.Background(), protoVmId)
	if err != nil {
		log.Printf("fail to dial: %v", err)
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
	log.Printf("Got assignments: assigned %v, cancelled: %v", assignedJobIds, cancelledJobIds)
	return &JobAssignments{
		assignedJobIds,
		cancelledJobIds,
	}
}

func getAssignedJobInfo(client pb_api.AgentApiSchedulerClient, vmId uint64, jobId uint64) *agent_api.JobInfo {
	log.Print("Getting assigned job info...")
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
		log.Printf("fail to dial: %v", err)
		return nil
	}
	jobInfo := agent_api.JobInfo{
		ImageVersion: *response.ImageVersion,
		Limits: agent_api.JobLimits{
			Cpu: *response.JobLimits.CpuCores,
			Ram: *response.JobLimits.MemoryMb,
		},
		TaskSettings: *response.TaskSettings,
		JobOptions:   *response.JobOptions,
	}
	log.Printf("Got assigned job info: %+v", jobInfo)
	return &jobInfo
}
