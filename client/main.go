package agent_api

import (
    "google.golang.org/grpc"
    "google.golang.org/grpc/grpclog"
    pb_api "scheduler/services"
)

func main() {
        opts := []grpc.DialOption{
            grpc.WithInsecure(),
        }
        //args := os.Args
        conn, err := grpc.Dial("127.0.0.1:50002", opts...)

        if err != nil {
            grpclog.Fatalf("fail to dial: %v", err)
        }

        defer conn.Close()

        vmId := uint64(42)
        client :=  pb_api.NewAgentApiSchedulerClient(conn)

        config := getConfig()
        state := State{}

        pollChan := pollSchedulerForAssignments(vmId, client, &config, &state)

        // ...

        close(pollChan)
}
