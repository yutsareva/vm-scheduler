## Docker for building the project in Clion IDE

```
docker build -t vm_scheduler:1.3 -f dev.Dockerfile .
docker run -d -t --cap-add sys_ptrace -p127.0.0.1:6348:22 --name vm_scheduler --memory="16g" vm_scheduler:1.3
```
Cmake flags: `-DCMAKE_TOOLCHAIN_FILE=/vcpkg/scripts/buildsystems/vcpkg.cmake`

## Main image

```
 docker-compose down --volumes && docker-compose up --build
```

## To run tests
- run postgres docker container
```
docker-compose down --volumes && docker-compose up --build db
```
- run tests
  - build docker "Docker for building the project in Clion IDE"
  - run gtests from bin/tests folder
