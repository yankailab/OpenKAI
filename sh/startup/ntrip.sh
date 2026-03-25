#!/bin/bash

while true; do
    /home/lab/dev/RTKLIB/app/consapp/str2str/gcc/str2str -in ntrip://USER:PASSWORD@CORS_HOST:2101/MOUNTPOINT -out ./corr.rtcm3

    sleep 1
done

