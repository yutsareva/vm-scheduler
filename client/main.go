package main

import (
	"log"
	"os"
	"os/signal"
	"scheduler/poller"
	"scheduler/registry"
	"scheduler/runner"
	"syscall"
)

func main() {
	log.SetFlags(log.LstdFlags)
	log.Print("Starting...")

	registry := registry.CreateRegistry()

	pollChan := poller.PollSchedulerForAssignments(registry)
	runJobsChan := runner.RunJobs(registry)
	log.Print("Started")

	finish := make(chan os.Signal)
	signal.Notify(finish, os.Interrupt, syscall.SIGTERM)
	<-finish

	close(pollChan)
	close(runJobsChan)

	registry.Close()
}
