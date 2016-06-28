#!/bin/bash
# autostart for streaming video on TX1
(
date
export PATH=$PATH:/bin:/sbin:/usr/bin:/usr/local/bin
export HOME=/home/ubuntu
cd $HOME/start_video
screen -d -m -S HttpVideo -s /bin/bash $HOME/start_video/start_http_video.sh
) > $HOME/start_video/start_http_video.log 2>&1
exit 0