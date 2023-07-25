#!/bin/bash

git clone https://github.com/yankailab/ardupilot
cd ardupilot
git submodule update --init --recursive

Tools/environment_install/install-prereqs-ubuntu.sh -y
. ~/.profile

./waf configure --board sitl
./waf copter
cd ~/ardupilot/ArduCopter

../Tools/autotest/sim_vehicle.py --map --console
Okushiga=36.779180,138.527999,1500,0
/home/kai/dev/ardupilot/Tools/autotest/sim_vehicle.py -v ArduCopter -f quad -L Okushiga --map --console --instance 1

./waf configure --board CubeBlack
./waf copter
./waf --targets bin/arducopter --upload
