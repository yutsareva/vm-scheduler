package registry

import (
	docker_client "github.com/docker/docker/client"
	"google.golang.org/grpc"
	"google.golang.org/grpc/grpclog"
	"log"
	"scheduler"
	"scheduler/s3_utils"
	pb_api "scheduler/services"
)

type Registry struct {
	Config Config
	State  State
	S3Manger s3_utils.S3Manager
	Client pb_api.AgentApiSchedulerClient
	DockerClient *docker_client.Client

	conn   *grpc.ClientConn
}

func (r *Registry) close() {
	r.conn.Close()
}

func createRegistry() *Registry {
	opts := []grpc.DialOption{
		grpc.WithInsecure(),
	}
	//args := os.Args
	conn, err := grpc.Dial("127.0.0.1:50002", opts...)

	if err != nil {
		grpclog.Fatalf("fail to dial: %v", err)
	}

	cli, err := docker_client.NewEnvClient()
	if err != nil {
		log.Fatalf("Failed to create docker cliet: %v", err)
	}

	return &Registry{
		Config:       agent_api.getConfig(),
		State:        State{},
		S3Manger:     s3_utils.CreateS3Manager(),
		Client:       pb_api.NewAgentApiSchedulerClient(conn),
		DockerClient: cli,
		conn:         conn,
	}
}