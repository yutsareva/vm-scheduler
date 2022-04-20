package registry

import "time"


type Config struct {
	PollInterval      time.Duration
	JobLaunchInterval time.Duration
	VmId              uint64
}

func getConfig() Config {
	return Config{
		PollInterval:      60 * time.Second,
		JobLaunchInterval: 5 * time.Second,
		VmId:              12345, // TODO
	}
}


