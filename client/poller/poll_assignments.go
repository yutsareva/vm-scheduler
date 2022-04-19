package poller

import (
	pb_api "scheduler/services"
	"time"
	agent_api "scheduler"
)

type JobAssignments struct {
	assignedJobIds []agent_api.JobId
	cancelledJobIds []agent_api.JobId
}

func PollSchedulerForAssignments(
		client pb_api.AgentApiSchedulerClient, config *agent_api.Config, state *agent_api.State) chan struct{} {
	ticker := time.NewTicker(config.PollInterval)
	quit := make(chan struct{})
	go func() {
		for {
			select {
			case <- ticker.C:
				assignments := getAssignments(config.VmId, client)
				if len(assignments.cancelledJobIds) > 0 {
					state.AddCancelledJobs(assignments.cancelledJobIds)
				}
				if len(assignments.assignedJobIds) > 0 {
					state.AddAssignedJobs(assignments.assignedJobIds)
				}
				assignedJobs := state.GetJobsForInfo()
				for _, jobId := range assignedJobs {
					maybeJobInfo := getAssignedJobInfo(client, config.VmId, uint64(jobId))
					if maybeJobInfo != nil {
						state.UpdateJobInfos(jobId, maybeJobInfo)
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



