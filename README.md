<p align="center">
<img src="https://github.com/yankailab/OpenKAI/raw/master/data/OK.png" width=300px alt="OpenKAI">
</p>

# OpenKAI: A lightweight & paralleled framework for unmanned vehicle and robot control
OpenKAI is an open-source framework that realize rapid prototyping & development of unmanned vehicles and robots.
It features a simple and unified multi-threading modular architecture which is purely written in C++.
OpenKAI is light weight to run on embedded hardwares yet is scalable to utilize the resources on high performance platforms.
OpenKAI is highly customizable for versatile applications and easily expanded to support new devices.

We are keep adding supports for sensors and actuators available on market from hackable modules to industry solutions.
The supported devices can be used easily as an abstracted object in OpenKAI.
We welcome contributions from vendors to provide evaluation device, as well as coders from open-source community.

## Platforms
Tested but not limited to
* x86 PC (>= Ubuntu 20.04 LTS, recommended 24.04 LTS)
* NVIDIA Jetson series (JetPack/Ubuntu)
* Raspberry Pi (Raspberry Pi OS)
* Kakip (Under testing)

## Dependencies
* Mandatory: Pthread, Google glog, Eigen.
* Optional: OpenCV, Open3D, OpenCL, etc..

## Interfaces
* UART
* USB
* CAN (incl. USB <-> CAN converter)
* RS-485 (USB <-> RS-485 converter)
* Ethernet

## Protocols
* Mavlink 2
* Modbus
* TCP/UDP
* WebSocket
* JSON

## External controllers
* Mavlink compatible flight controllers
* Oriental Motor (Modbus/RS-485)
* ZLAC 8015 (Modbus/RS-485)

## Sensors
* Intel Realsense.
* Orbbec (Coming soon)
* Livox
* LeddarTech LeddarVu
* TOFsense
* Raspberry Camera module

## Actuators
* Oriental Motor
* DRV8825

## Showcase of Solutions
Coming soon.


# Functions


# Setup & Install
Refer to [setup](./docs/setup.md).
