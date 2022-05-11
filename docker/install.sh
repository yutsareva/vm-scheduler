set -e -x

apt-get update && \
    apt-get install -y \
      cmake curl zip unzip tar libpqxx-dev && \
    git clone https://github.com/microsoft/vcpkg && \
  ./vcpkg/bootstrap-vcpkg.sh  && \
  ./vcpkg/vcpkg install "zkpp" --recurse
  ./vcpkg/vcpkg install "aws-sdk-cpp[ec2]" --recurse  && \
  ./vcpkg/vcpkg install "protobuf" --recurse  && \
  ./vcpkg/vcpkg install "grpc" --recurse && \
  ./vcpkg/vcpkg install "rapidjson" --recurse
