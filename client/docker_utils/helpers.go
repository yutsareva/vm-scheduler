package docker_utils

import (
	"context"
	"github.com/docker/docker/api/types"
	"github.com/docker/docker/api/types/container"
	"github.com/docker/docker/api/types/mount"
	"github.com/docker/docker/client"
	"io"
	"os"
	//"github.com/docker/go-connections/nat"
)

func PullImage(ctx context.Context, cli *client.Client, image *string) error {
	reader, err := cli.ImagePull(ctx, *image, types.ImagePullOptions{})
	if err != nil {
		return err
	}
	io.Copy(os.Stdout, reader) // TODO
	return nil
}

func CreateContainer(ctx context.Context, cli *client.Client, jobName *string, imageVersion *string) (*string, error) {
	resp, err := cli.ContainerCreate(ctx, &container.Config{
		Image: *imageVersion,
		//Volumes: map[string]struct{}{jobName{}},
		Cmd: []string{
			"--setting", "/task/settings.json",
			"--job-options", "/task/job_options.json",
			"--output", "/task/result.json",
		},
	},
		&container.HostConfig{
			Mounts: []mount.Mount{
				{
					Type:   mount.TypeBind,
					Source: "/" + *jobName,
					Target: "/task",
				},
			},
		}, nil, nil, *jobName)
	if err != nil {
		return nil, err
	}
	return &resp.ID, nil
}

func StartContainer(ctx context.Context, cli *client.Client, containerId *string) error {
	return cli.ContainerStart(ctx, *containerId, types.ContainerStartOptions{})
}
