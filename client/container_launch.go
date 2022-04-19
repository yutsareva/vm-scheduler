package agent_api

import (
	"context"
	"fmt"
	"github.com/docker/docker/api/types/mount"
	"io"
	"os"

	"github.com/docker/docker/api/types"
	"github.com/docker/docker/api/types/container"
	"github.com/docker/docker/client"
	//"github.com/docker/go-connections/nat"
)

func pullImage(ctx context.Context, cli *client.Client, image *string) error {
	reader, err := cli.ImagePull(ctx, *image, types.ImagePullOptions{})
	if err != nil {
		return err
	}
	io.Copy(os.Stdout, reader) // TODO
	return nil
}

func createContainer(ctx context.Context, cli *client.Client, jobName *string, jobInfo *JobInfo) (*string, error) {
	resp, err := cli.ContainerCreate(ctx, &container.Config{
		Image:        jobInfo.imageVersion,
		//Volumes: map[string]struct{}{jobName{}},
		Cmd:   []string{
			"--setting", "/task/settings.json",
			"--job-options", "/task/job_options.json",
			"--output", "/task/result.json",
		},
	},
	&container.HostConfig{
		Mounts: []mount.Mount{
			{
				Type:   mount.TypeBind,
				Source: "/"+*jobName,
				Target: "/task",
			},
		},
	}, nil, nil, *jobName)
	if err != nil {
		return nil, err
	}
	return &resp.ID, nil
}

func startContainer(ctx context.Context, cli *client.Client, containerId *string) error {
	return cli.ContainerStart(ctx, *containerId, types.ContainerStartOptions{})
}

func runJobContainer(jobId JobId, jobInfo *JobInfo) (*string, error) {
	cli, err := client.NewEnvClient()
	if err != nil {
		return nil, err
	}

	jobName := fmt.Sprintf("job-%d", jobId)

	ctx := context.Background()
	err = pullImage(ctx, cli, &jobInfo.imageVersion)
	if err != nil {
		return nil, err
	}

	containerId, err := createContainer(ctx, cli, &jobName, jobInfo)
	if err != nil {
		return nil, err
	}

	err = startContainer(ctx, cli, containerId)
	if err != nil {
		return nil, err
	}

	//go handleContainerCompletion(ctx, cli, containerId)

	return containerId, nil
}
