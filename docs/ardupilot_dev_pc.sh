#!/bin/bash

git clone --depth 1 https://github.com/ArduPilot/ardupilot.git
cd ardupilot
git submodule update --init --recursive

Tools/environment_install/install-prereqs-ubuntu.sh -y
. ~/.profile

./waf configure --board sitl
./waf copter
cd ArduCopter

./Tools/autotest/sim_vehicle.py -v ArduCopter -f quad --map --console
nano ardupilot/Tools/autotest/locations.txt
Okushiga=36.779180,138.527999,1500,0
/home/kai/dev/ardupilot/Tools/autotest/sim_vehicle.py -v ArduCopter -f quad -L Okushiga --map --console --count=1 --add-param-file=/home/kai/ap_sitl_copter.parm

./waf configure --board CubeBlack
./waf copter
./waf --targets bin/arducopter --upload
