package agent_api

import "time"


type Config struct {
	pollInterval time.Duration
	vmId uint64
}

func getConfig() Config {
	return Config {
		pollInterval: 60 * time.Second,
		vmId: 12345, // TODO
	}
}


