#!/bin/bash
set -ex

#sudo usermod -aG docker $USER
#sudo chown "$USER" /home/"$USER"/.docker -R
#sudo chmod g+rwx "$HOME/.docker" -R

export VERS=latest
export IMAGE=545868914688.dkr.ecr.us-east-2.amazonaws.com/vm-scheduler/agent:$VERS

aws configure set aws_access_key_id $AWS_ACCESS_KEY_ID
aws configure set aws_secret_access_key $AWS_SECRET_ACCESS_KEY
aws configure set default.region us-east-2
aws configure set default.output json

aws ecr get-login-password --region us-east-2 | docker login --username AWS --password-stdin 545868914688.dkr.ecr.us-east-2.amazonaws.com

docker build --network=host -t $IMAGE -f agent.Dockerfile ../
docker push $IMAGE
