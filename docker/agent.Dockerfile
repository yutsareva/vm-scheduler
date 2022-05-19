FROM golang:1.18

WORKDIR /app
COPY . .

RUN wget https://s3.eu-central-1.amazonaws.com/amazoncloudwatch-agent-eu-central-1/ubuntu/amd64/latest/amazon-cloudwatch-agent.deb && \
    dpkg -i -E ./amazon-cloudwatch-agent.deb && \
    rm ./amazon-cloudwatch-agent.deb \

RUN apt-get update && apt-get install -y protobuf-compiler && \
    go install google.golang.org/protobuf/cmd/protoc-gen-go@v1.28 && \
    go install google.golang.org/grpc/cmd/protoc-gen-go-grpc@v1.2

RUN cd proto && protoc --go_out=../client --go_opt=paths=source_relative \
  --go-grpc_out=../client --go-grpc_opt=paths=source_relative **/*.proto


RUN cd client && go mod tidy && go build

RUN apt-get install -y supervisor

ADD client/supervisord.conf /etc/supervisor/conf.d/supervisord.conf
ADD client/amazon-cloudwatch-agent.json /opt/aws/amazon-cloudwatch-agent/etc/amazon-cloudwatch-agent.json

ENTRYPOINT ["/usr/bin/supervisord"]
