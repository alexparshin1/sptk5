
# Source: https://hub.docker.com/r/gvenzl/oracle-xe

docker pull gvenzl/oracle-xe

# Start
#docker run -d -p 8080:8080 -p 1521:1521 -e ORACLE_PASSWORD=oracle -v /home/alexeyp/docker_data/oracle:/opt/oracle/oradata gvenzl/oracle-xe -name oracle

docker run -d -p 1521:1521 -e ORACLE_PASSWORD=oracle -v /home/alexeyp/docker_data/oracle:/opt/oracle/oradata gvenzl/oracle-xe

echo now connect to DB 
echo user: sys as sysdba
echo password: oracle
echo command: ALTER SYSTEM DISABLE RESTRICTED SESSION; 