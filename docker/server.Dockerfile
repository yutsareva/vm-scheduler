FROM gcc:latest

COPY docker/install.sh ./
RUN sh install.sh

# Notice: context is parent dir
ADD bin /app/bin
ADD libs /app/libs
ADD proto /app/proto
ADD CMakeLists.txt /app/CMakeLists.txt

WORKDIR /app/build

RUN cmake -DCMAKE_TOOLCHAIN_FILE=/vcpkg/scripts/buildsystems/vcpkg.cmake ../ && \
    cmake --build . --target server

ENTRYPOINT ["./bin/server"]
