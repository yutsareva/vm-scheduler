package agent_api

import "time"


type Config struct {
	pollInterval time.Duration
}

func getConfig() Config {
	return Config {
		pollInterval: 60 * time.Second,
	}
}


