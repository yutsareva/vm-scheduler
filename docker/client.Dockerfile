protoc --go_out=../client --go_opt=paths=source_relative \
     --go-grpc_out=../client --go-grpc_opt=paths=source_relative **/*.proto
