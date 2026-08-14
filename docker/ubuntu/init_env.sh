echo $LD_LIBRARY_PATH | grep instantclient > /dev/null

[ $? = 0 ] && exit 0

export LD_LIBRARY_PATH=/usr/local/lib:/usr/local/lib64:/opt/oracle/instantclient_18_3:${LD_LIBRARY_PATH}
grep "10.1.1.242" /etc/hosts
if [ $? == 1 ]; then
    echo "10.1.1.242  theater oracledb dbhost_oracle dbhost_mssql dbhost_pg dbhost_mysql smtp_host redis_server mosquitto_server" >> /etc/hosts
fi
