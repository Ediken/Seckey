#!/bin/bash
# 停止密钥协商服务端
# 用法: ./stopSvr.sh

PID=$(ps -ef | grep seckey_server | grep -v grep | awk '{print $2}')
if [ -n "$PID" ]; then
    kill -USR1 $PID   # 发送 SIGUSR1（第 9 天：优雅退出）
    echo "已发送停止信号给 PID=$PID"
else
    echo "seckey_server 未在运行"
fi
