#!/bin/sh

DOCKER_DATA=/home/alexeyp/docker_data

docker exec -e 'ACCEPT_EULA=Y' -e 'SA_PASSWORD=Wsxedc88' -it protis-mssql /var/opt/mssql/scripts/restore.sh

