#!/bin/bash
# 启动密钥协商服务端（守护进程方式）
# 用法: ./startSvr.sh

cd /home/lero/project/project_first/seckey

# 如果已经在运行，先提示
PID=$(ps -ef | grep seckey_server | grep -v grep | awk '{print $2}')
if [ -n "$PID" ]; then
    echo "seckey_server 已在运行: PID=$PID"
    exit 0
fi

# 启动（nohup + & 让它在后台跑，日志写进 svr.log）
nohup ./build/bin/seckey_server > svr.log 2>&1 &

echo "seckey_server 已启动, 日志: svr.log"
