package agent_api

import(
	"sync"
)

type JobId uint64

type State struct {
	mu       sync.Mutex
	assignedJobs []JobId
	cancelledJobs []JobId
	runningJobs []JobId
}

func (s *State) updateAssignments(jobIds []JobId) {
	s.mu.Lock()
	defer s.mu.Unlock()

	s.assignedJobs = append(s.assignedJobs, jobIds...)
}
