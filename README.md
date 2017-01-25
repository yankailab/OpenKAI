# OpenKAI

## What is it?
OpenKAI (Kinetic AI) is a framework that combines AI and robot controllers. OpenKAI is designed to be highly customizable for versatile applications, yet light weight to be ran on embedded hardwares, and a simple code architecture that is easy for expansion and maintenance.

## Platform
OpenKAI is supposed to behave as a companion computer that commands an external low-level robotic controller. Supported hardware for the companiton computer currently are
* x86 PC & Ubuntu (14.04, 16.04)
* NVIDIA JetsonTX1 & Ubuntu (JetPack 2.3.1)

## External controller
* Pixhawk (via Mavlink/UART)
* Controllers with CAN bus I/F (via UART/USB<->CAN bus converter)

## Sample apps
Visual obstacle avoidance using ZED camera and Pixhawk (APMcopter 3.5-dev and above).

On-board camera (Click to jump to YouTube):

[![APMcopter+OpenKAI+ZED: visual obstacle avoidance](https://img.youtube.com/vi/MOFullt5k3g/0.jpg)](https://www.youtube.com/watch?v=MOFullt5k3g)

External camera (Click to jump to YouTube):

[![APMcopter+OpenKAI+ZED: visual obstacle avoidance](https://img.youtube.com/vi/qk_hEtRASqg/0.jpg)](https://www.youtube.com/watch?v=qk_hEtRASqg)


## Build & Run
###[Build on NVIDIA Jetson TX1](https://github.com/yankailab/OpenKAI/blob/master/doc/JetsonTX1/build.md)

###[Build on Ubuntu desktop systems](https://github.com/yankailab/OpenKAI/blob/master/doc/x86_64/Ubuntu16.04LTS/build.md)
  
## System architecture
OpenKAI is organized as a combination of multiple functional Modules. Each Module runs in an individual thread, so that modules can be ran simultaneously over multi-core CPUs efficiently. Each Module contains multiple sub-Modules, each sub-Module may handle certain part of functions divided by its category, and the Module switches between each of its sub-Modules to form a complete function.
<p align="center">
<img src="https://github.com/yankailab/OpenKAI/raw/master/doc/img/F1.png" alt="OpenKAI Modules" width="600px">
</p>
At source code level, each Module and sub-Module is implemented by an individual C++ class. A Module class is inherited from kai::ThreadBase class that handles thread interface and timing control (desired frames per second) etc.

A typical example of Modular designed OpenKAI system for an automatic visual guided landing system is shown below.
<p align="center">
<img src="https://github.com/yankailab/OpenKAI/raw/master/doc/img/F2.png" alt="OpenKAI autotmatic visual guided landing system diagram" width="800px">
</p>
Refer [here](http://ardupilot.org/dev/docs/companion-computer-nvidia-tx1.html) for hardware connections of the above example, and [here](https://youtu.be/5AVb2hA2EUs) is a video showing the example in action.

The overall system implemented by OpenKAI is a single-process multi-threads program. This architecture aims to provide an easy expansion similar to ROS, meanwhile keeping the program to be simple and provide an efficient interface between Modules by eliminating the socket communication in ROS. A reference of currently implemented Modules and sub-Modules will be provided soon.

## Application implementation
There are two ways of implement an application based on OpenKAI: compilation vs. script.

### Compilation
OpenKAI is implemented purely in C++, it is very easy to implement your application as C/C++ and compile with OpenKAI together using such as cmake build tool or IDEs such as Eclipse CDT. Refer to application/general.cpp as a starting point for application integration. A tutorial on setting up Eclipse CDT with OpenKAI will be provided soon.

### Kiss (OpenKai Simple Script)
If the existing function Modules from OpenKAI already satisfy your needs, you can write a json-like Kiss script to define your application. Note that Kiss is treated as a config file and will be parsed all at once at the beginning of OpenKAI program execution. The Modules defined by Kiss file are statically created and get started to run. Therefore there is no difference in execution speed between the Compilation way and application defined by Kiss.

To be updated.
