<p align="center">
<img src="https://github.com/yankailab/OpenKAI/raw/master/data/OK.png" alt="OpenKAI">
</p>

## OpenKAI: A modern framework for unmanned vehicle and robot control
OpenKAI (Kinetic AI) is an open-source framework that supports rapid development of unmanned vehicles and robots that incorporate AI, varied sensors, actuators etc.. OpenKAI is light weight to run on embedded hardwares. A simple and unified multi-threading modular architecture written purely in C++ makes it highly customizable for versatile applications, easy for expansion and maintenance.
We are keep adding supports for sensors and actuators available on market from hacker products to industry solutions. The supported devices can be used easily as an abstracted object in OpenKAI. We welcome contributions from device vendors to provide evaluation device, as well as coders from open-source community.

## Platforms
Tested but not limited to
* x86 PC (Ubuntu 20.04 LTS)
* NVIDIA Jetson series (JetPack/Ubuntu)
* Raspberry Pi (Raspberry Pi OS)

## Dependencies
A small set of library are choosen carefully for the minimum denpendency
Compulsory:
Pthread, Google glog, Eigen.
Optional:
OpenCV, Open3D, OpenCL, etc..

## Interfaces
* UART
* USB Serial
* CAN (incl. USB <-> CAN converter)
* RS-485 (USB <-> RS-485 converter)
* LAN/Wifi

## Protocols
* Mavlink 2
* Modbus
* TCP/UDP
* WebSocket
* JSON

## External controllers
* Pixhawk and its compatibles (Mavlink/UART)
* Oriental Motor (Modbus/RS-485)
* ZLAC 8015 (Modbus/RS-485)
and more are coming.

## Sensors
* Intel Realsense D400 series, L515, T265.
* LeddarTech LeddarVu (UART/USB)
* TOFsense
* Raspberry Camera module
* Hiphen Airphen
* Canon SLR
* Sony RX0 series
* Livox (Coming soon)
and more are coming.

## Actuators
* Oriental Motor
* DRV8825 (RS485)
and more are coming.

## Displays
* Mutrics ARiC AR glasses (Coming soon)
* Ordinal Displays

## Showcase of Solutions
Visual obstacle avoidance using ZED camera and Pixhawk (APMcopter 3.5-dev and above).

On-board camera (Click to jump to YouTube):

[![APMcopter+OpenKAI+ZED: visual obstacle avoidance](https://img.youtube.com/vi/MOFullt5k3g/0.jpg)](https://www.youtube.com/watch?v=MOFullt5k3g)

External camera (Click to jump to YouTube):

[![APMcopter+OpenKAI+ZED: visual obstacle avoidance](https://img.youtube.com/vi/qk_hEtRASqg/0.jpg)](https://www.youtube.com/watch?v=qk_hEtRASqg)

## Getting started
###[Set up and compilation](https://github.com/yankailab/OpenKAI/blob/master/sh/Setup/OpenKAI_dev_setup.sh)
Updating...
