FROM golang:1.18

WORKDIR /app
COPY . .

RUN apt-get update && apt-get install -y protobuf-compiler && \
    go install google.golang.org/protobuf/cmd/protoc-gen-go@v1.28 && \
    go install google.golang.org/grpc/cmd/protoc-gen-go-grpc@v1.2

RUN cd proto && protoc --go_out=../client --go_opt=paths=source_relative \
  --go-grpc_out=../client --go-grpc_opt=paths=source_relative **/*.proto


RUN cd client && go mod tidy && go build

CMD ["/app/client/scheduler"]
