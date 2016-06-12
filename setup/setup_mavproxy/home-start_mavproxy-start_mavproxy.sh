#!/bin/sh
mavproxy.py --master /dev/ttyS0 --baud 921600 --out udpin:localhost:9000 --out udpbcast:10.0.1.255:14550
