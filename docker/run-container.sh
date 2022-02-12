docker build -t vm_scheduler:1.1 -f Dockerfile .

docker run -d -t --cap-add sys_ptrace -p127.0.0.1:6348:22 --name vm_scheduler --memory="16g" vm_scheduler:1.1
