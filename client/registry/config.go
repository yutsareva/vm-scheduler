package registry

import (
	"log"
	"os"
	"strconv"
	"time"
)

type Config struct {
	PollInterval      time.Duration
	JobLaunchInterval time.Duration
	VmId              uint64
}

func getConfig() Config {
	vmId, err := strconv.ParseUint(os.Getenv("VMS_AGENT_ID"), 10, 64)
	if err != nil {
		log.Fatalf("Invalid vm id: %v", err)
	}
	log.Printf("VM ID: %d", vmId)
	return Config{
		PollInterval:      5 * time.Second,
		JobLaunchInterval: 5 * time.Second,
		VmId:              vmId,
	}
}
