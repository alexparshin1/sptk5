#!/bin/sh

# The databases the unit tests use, kept running: --restart=unless-stopped brings them back after a crash
# or a reboot. With --rm a container that stopped for any reason was gone with its name (2026-10-14: the
# night build failed every Linux image on "ORA-12541: TNS:no listener"). The farm checks them and starts
# what is missing before it builds: xmq_build_farm/ensure_databases.sh.
DOCKER_DATA=/home/alexeyp/docker_data

mkdir -p $DOCKER_DATA
cd $DOCKER_DATA
mkdir -p mssql oracle

docker run --name=oracle -d --restart=unless-stopped -p 1521:1521 -p 8080:8080 -e ORACLE_PASSWORD=oracle -v $DOCKER_DATA/oracle:/opt/oracle/oradata gvenzl/oracle-xe
docker run --name mssql -d --restart=unless-stopped -e "ACCEPT_EULA=Y" -e "MSSQL_SA_PASSWORD=Sl0nic#757" -v $DOCKER_DATA/mssql:/var/opt/mssql -p 1433:1433 --hostname mssql mcr.microsoft.com/mssql/server:2022-latest

#docker run --name emqx --rm -d -p 18083:18083 -p 1883:1883 -p 8883:8883 emqx:latest
#docker run --name redis --rm -d -p 6379:6379 -p 8001:8001 redis:latest

#docker run --name=jenkins --rm -p 9000:8080 -p 50000:50000 --restart=on-failure jenkins/jenkins:lts-jdk21 > jenkins.txt
