
# Source: https://hub.docker.com/r/gvenzl/oracle-xe

docker pull mcr.microsoft.com/mssql/server:2022-latest

docker run --name mssql --rm -e "ACCEPT_EULA=Y" -e "MSSQL_SA_PASSWORD=Sl0nic#757" -v $DOCKER_DATA/mssql:/var/opt/mssql -p 1433:1433 --hostname mssql -d mcr.microsoft.com/mssql/server:2022-latest
