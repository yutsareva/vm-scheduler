package agent_api

import (
    "google.golang.org/grpc"
    "google.golang.org/grpc/grpclog"
    "log"
    "scheduler/poller"
    pb_api "scheduler/services"
)

func main() {
        log.SetFlags(log.LstdFlags)
        opts := []grpc.DialOption{
            grpc.WithInsecure(),
        }
        //args := os.Args
        conn, err := grpc.Dial("127.0.0.1:50002", opts...)

        if err != nil {
            grpclog.Fatalf("fail to dial: %v", err)
        }

        defer conn.Close()

        client :=  pb_api.NewAgentApiSchedulerClient(conn)

        config := getConfig()
        state := State{}

        pollChan := poller.PollSchedulerForAssignments(client, &config, &state)

        // ...

        close(pollChan)
}
