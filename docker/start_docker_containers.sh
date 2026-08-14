#!/bin/sh

DOCKER_DATA=/home/alexeyp/docker_data

mkdir -p $DOCKER_DATA
cd $DOCKER_DATA
mkdir -p mssql oracle

docker run --name=oracle --rm -d -p 1521:1521 -p 8080:8080 -e ORACLE_PASSWORD=oracle -v $DOCKER_DATA/oracle:/opt/oracle/oradata gvenzl/oracle-xe
docker run --name mssql --rm -e "ACCEPT_EULA=Y" -e "MSSQL_SA_PASSWORD=Sl0nic#757" -v $DOCKER_DATA/mssql:/var/opt/mssql -p 1433:1433 --hostname mssql -d mcr.microsoft.com/mssql/server:2022-latest

#docker run --name emqx --rm -d -p 18083:18083 -p 1883:1883 -p 8883:8883 emqx:latest
#docker run --name redis --rm -d -p 6379:6379 -p 8001:8001 redis:latest

#docker run --name=jenkins --rm -p 9000:8080 -p 50000:50000 --restart=on-failure jenkins/jenkins:lts-jdk21 > jenkins.txt
