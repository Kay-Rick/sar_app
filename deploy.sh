#!/bin/bash
NAME="sar_server"
# If "main" process is running, we kill it before restart it.
PROCESS=`ps -ef | grep ${NAME} |grep -v grep | awk '{print $1}'`
for pid in $PROCESS
do
    echo "Kill process $pid"
    kill -9 $pid
done

./sar_server 192.168.1.191 10
