#!/bin/bash

git clone https://github.com/yankailab/ardupilot
cd ardupilot
git submodule update --init --recursive

Tools/environment_install/install-prereqs-ubuntu.sh -y
. ~/.profile

./waf configure --board CubeBlack
./waf copter
./waf --targets bin/arducopter --upload
