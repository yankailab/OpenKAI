#!/bin/bash
# autostart for mavproxy on TX1

(
date
export PATH=$PATH:/bin:/sbin:/usr/bin:/usr/local/bin
export HOME=/home/ubuntu
cd $HOME/start_mavproxy
screen -d -m -S MAVProxy -s /bin/bash $HOME/start_mavproxy/start_mavproxy.sh
) > $HOME/start_mavproxy/start_mavproxy.log 2>&1
exit 0
