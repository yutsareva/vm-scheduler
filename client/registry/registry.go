package registry

import (
	docker_client "github.com/docker/docker/client"
	"google.golang.org/grpc"
	"google.golang.org/grpc/grpclog"
	"log"
	"os"
	"scheduler/s3_utils"
	pb_api "scheduler/services"
)

type Registry struct {
	Config       Config
	State        State
	S3Manger     s3_utils.S3Manager
	Client       pb_api.AgentApiSchedulerClient
	DockerClient *docker_client.Client

	conn *grpc.ClientConn
}

func (r *Registry) Close() {
	r.conn.Close()
}

func CreateRegistry() *Registry {
	opts := []grpc.DialOption{
		grpc.WithInsecure(),
	}
	//args := os.Args
	grpcServerAddress := os.Getenv("VMS_AGENT_SCHEDULER_ADDRESS")
	conn, err := grpc.Dial(grpcServerAddress, opts...)

	if err != nil {
		grpclog.Fatalf("fail to dial: %v", err)
	}

	cli, err := docker_client.NewEnvClient()
	if err != nil {
		log.Fatalf("Failed to create docker cliet: %v", err)
	}

	return &Registry{
		Config:       getConfig(),
		State:        State{},
		S3Manger:     s3_utils.CreateS3Manager(),
		Client:       pb_api.NewAgentApiSchedulerClient(conn),
		DockerClient: cli,
		conn:         conn,
	}
}
