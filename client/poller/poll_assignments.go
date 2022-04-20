package poller

import (
	"scheduler/registry"
	"time"
)

type JobAssignments struct {
	assignedJobIds  []registry.JobId
	cancelledJobIds []registry.JobId
}

func PollSchedulerForAssignments(
	registry *registry.Registry) chan struct{} {
	ticker := time.NewTicker(registry.Config.PollInterval)
	quit := make(chan struct{})
	go func() {
		for {
			select {
			case <-ticker.C:
				assignments := getAssignments(registry.Config.VmId, registry.Client)
				if len(assignments.cancelledJobIds) > 0 {
					registry.State.AddCancelledJobs(assignments.cancelledJobIds)
				}
				if len(assignments.assignedJobIds) > 0 {
					registry.State.AddAssignedJobs(assignments.assignedJobIds)
				}
				assignedJobs := registry.State.GetJobsForInfo()
				for _, jobId := range assignedJobs {
					maybeJobInfo := getAssignedJobInfo(registry.Client, registry.Config.VmId, uint64(jobId))
					if maybeJobInfo != nil {
						registry.State.UpdateJobInfos(jobId, maybeJobInfo)
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
