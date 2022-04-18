package agent_api

import(
	"sync"
)

type JobId uint64

type JobLimits struct {
	cpu uint32
	ram uint32
}

type JobInfo struct {
	imageVersion string
	limits JobLimits
	taskSettings string
	jobOptions string
}

type JobResult struct {
	resultUrl string
	// status TODO
}

type void struct{}
var member void

type State struct {
	mu       sync.Mutex

	assignedJobs map[JobId]void
	readyToRunJobs map[JobId]void
	cancelledJobs map[JobId]void
	runningJobs map[JobId]void
	completedJobs map[JobId]JobResult

	jobIdToInfo map[JobId]*JobInfo
}

func (s *State) addAssignedJobs(jobIds []JobId) {
	s.mu.Lock()
	defer s.mu.Unlock()

	for _, jobId := range jobIds {
		if _, ok := s.readyToRunJobs[jobId]; !ok {
			s.assignedJobs[jobId] = member
		}
	}
}

func (s *State) getJobsForInfo() []JobId {
	s.mu.Lock()
	defer s.mu.Unlock()

	keys := make([]JobId, len(s.assignedJobs))

	i := 0
	for k := range s.assignedJobs {
		keys[i] = k
		i++
	}
	return keys
}

func (s *State) updateJobInfos(id JobId, info *JobInfo) {
	s.mu.Lock()
	defer s.mu.Unlock()

	s.jobIdToInfo[id] = info
	delete(s.assignedJobs, id)
	s.readyToRunJobs[id] = member
}

func (s *State) addCancelledJobs(jobIds []JobId) {
	s.mu.Lock()
	defer s.mu.Unlock()

	for _, jobId := range jobIds {
		s.cancelledJobs[jobId] = member
	}
}

func (s *State) cancelJobs(jobIds []JobId) {
	s.mu.Lock()
	defer s.mu.Unlock()

	for _, jobId := range jobIds {
		delete(s.cancelledJobs, jobId)
		delete(s.jobIdToInfo, jobId)
	}
}

func (s *State) getReadyToRunJob() (*JobId, *JobInfo) {
	s.mu.Lock()
	defer s.mu.Unlock()

	for jobId, _ := range s.readyToRunJobs {
		delete(s.readyToRunJobs, jobId)
		s.runningJobs[jobId] = member
		return &jobId, s.jobIdToInfo[jobId]
	}
	return nil, nil
}

func (s *State) completeJob(jobId JobId, jobResult JobResult) {
	s.mu.Lock()
	defer s.mu.Unlock()

	delete(s.runningJobs, jobId)
	s.completedJobs[jobId] = jobResult
}

func (s *State) removeJob(jobId JobId) {
	s.mu.Lock()
	defer s.mu.Unlock()

	delete(s.completedJobs, jobId)
	delete(s.jobIdToInfo, jobId)
}

