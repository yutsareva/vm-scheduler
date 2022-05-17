## How to run ZooKeeper cluster in AWS

Instructions: https://medium.com/@pacuna/running-a-zookeeper-ensemble-on-aws-8025a66e0c1:
Binary: https://archive.apache.org/dist/zookeeper/zookeeper-3.4.12/zookeeper-3.4.12.tar.gz

```bash
wget https://archive.apache.org/dist/zookeeper/zookeeper-3.4.12/zookeeper-3.4.12.tar.gz
tar -zxf zookeeper-3.4.12.tar.gz
sudo mv zookeeper-3.4.12 /usr/local/zookeeper
sudo mkdir /var/lib/zookeeper
echo "tickTime=2000
      initLimit=10
      syncLimit=5
      dataDir=/var/lib/zookeeper
      clientPort=2181
      server.1=172.31.36.127:2888:3888
      server.2=172.31.47.132:2888:3888
      server.3=172.31.43.26:2888:38881" > /usr/local/zookeeper/conf/zoo.cfg
// replace N with 1, 2 or 3
echo N | sudo tee -a /var/lib/zookeeper/myid
```
// Fix security group rules for every instance (if they have different security groups)
```bash
sudo /usr/local/zookeeper/bin/zkServer.sh start
```
