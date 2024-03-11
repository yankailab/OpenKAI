#!/bin/bash

#----------------------------------------------------
# (Optional) Update kernel if needed
uname -r
wget https://kernel.ubuntu.com/mainline/v6.6.7/amd64/linux-headers-6.6.7-060607-generic_6.6.7-060607.202312131837_amd64.deb
wget https://kernel.ubuntu.com/mainline/v6.6.7/amd64/linux-headers-6.6.7-060607_6.6.7-060607.202312131837_all.deb
wget https://kernel.ubuntu.com/mainline/v6.6.7/amd64/linux-image-unsigned-6.6.7-060607-generic_6.6.7-060607.202312131837_amd64.deb
wget https://kernel.ubuntu.com/mainline/v6.6.7/amd64/linux-modules-6.6.7-060607-generic_6.6.7-060607.202312131837_amd64.deb
sudo dpkg -i *.deb
sudo reboot now

#----------------------------------------------------
# (Optional) sshd
sudo apt-get install ssh
systemctl start sshd

# (Optional) opc
sudo apt-get install openconnect
#sudo apt install network-manager-openconnect-gnome
sudo openconnect -u username -b server.com
ip a

#----------------------------------------------------
# System setup
# Install Ubuntu without Download updates during install checked
sudo apt-get update
sudo apt-get upgrade
sudo apt-get --with-new-pkgs upgrade
sudo aptitude full-upgrade
sudo apt-get update
sudo apt-get upgrade
#sudo apt-get dist-upgrade

# Basic
sudo apt-get -y install build-essential clang libc++-dev libc++abi-dev cmake cmake-curses-gui ninja-build git autoconf automake libtool pkg-config libssl-dev libboost-all-dev libgflags-dev uuid-dev libboost-filesystem-dev libboost-system-dev libboost-thread-dev ncurses-dev libssl-dev libprotobuf-dev protobuf-compiler libcurl4 curl libusb-1.0-0-dev libusb-dev libudev-dev libc++-dev libc++abi-dev libfmt-dev

# (Optional) Update GCC
sudo apt-get -y install g++-11 gcc-11
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 9 --slave /usr/bin/g++ g++ /usr/bin/g++-9
sudo update-alternatives --config gcc

# Image, codecs, gstreamer
sudo apt-get -y install libunwind-dev
sudo apt-get -y install gstreamer1.0-0 gstreamer1.0-plugins-base libgstreamer1.0-0 libgstreamer-plugins-base1.0-dev gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-alsa gstreamer1.0-gl libv4l-dev v4l-utils libjpeg-dev libpng-dev libtiff-dev libavcodec-dev libavformat-dev libxvidcore-dev x264

# OpenGL
sudo apt-get -y install libglu1-mesa-dev libglu1-mesa libgl1-mesa-glx libgl1-mesa-dev libglfw3 libglfw3-dev libglew-dev mesa-common-dev freeglut3-dev xorg-dev libxt-dev libxi-dev libx11-dev

# Desktop
sudo apt-get -y install libgtk-3-dev gstreamer1.0-x gstreamer1.0-gtk3

# Numerical
sudo apt-get -y install libqhull-dev qhull-bin gfortran libblas-dev liblapack-dev liblapacke-dev liblapack3 libatlas-base-dev libopenblas-base libopenblas-dev gsl-bin libgsl0-dev libflann-dev libproj-dev

# Graph plotting
sudo apt-get -y install libmgl-dev

# (Optional)
sudo apt-get -y install libimage-exiftool-perl
sudo apt-get -y install python python3-pip
pip install --upgrade google-api-python-client google-auth-httplib2 google-auth-oauthlib
snap install cloudcompare

sudo apt autoremove
sudo apt clean

#----------------------------------------------------
# (Optional) CUDA
# Update the video driver first with Software and Update
wget https://developer.download.nvidia.com/compute/cuda/12.3.2/local_installers/cuda_12.3.2_545.23.08_linux.run
sudo chmod a+x cuda_12.3.2_545.23.08_linux.run
sudo sh cuda_12.3.2_545.23.08_linux.run
sudo echo -e "export PATH=/usr/local/cuda-12.3/bin:\$PATH\nexport LD_LIBRARY_PATH=/usr/local/cuda-12.3/lib64:\$LD_LIBRARY_PATH\nexport LC_ALL=en_US.UTF-8" >> ~/.bashrc

#----------------------------------------------------
# CMake
wget https://github.com/Kitware/CMake/releases/download/v3.25.2/cmake-3.25.2.tar.gz
tar xvf cmake-3.25.2.tar.gz
cd cmake-3.25.2
./bootstrap
make -j$(nproc)
sudo make install
sudo reboot now

#----------------------------------------------------
# Eigen
wget https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz
tar xvf eigen-3.4.0.tar.gz
cd eigen-3.4.0
mkdir build
cd build
cmake ../
sudo make install

#----------------------------------------------------
# (Optional) RealSense
git clone --depth 1 https://github.com/IntelRealSense/librealsense.git
# git clone --depth 1 --branch v2.53.1 https://github.com/IntelRealSense/librealsense.git # for T265, L535
cd librealsense
sudo ./scripts/setup_udev_rules.sh
mkdir build
cd build
# for Raspberry pi
cmake -DFORCE_LIBUVC=true -DFORCE_RSUSB_BACKEND=ON -DBUILD_WITH_TM2=true -DBUILD_WITH_CUDA=OFF -DCMAKE_BUILD_TYPE=Release -DIMPORT_DEPTH_CAM_FW=ON ../
# for Jetson
cmake -DFORCE_LIBUVC=true -DFORCE_RSUSB_BACKEND=ON -DBUILD_WITH_TM2=true -DBUILD_WITH_CUDA=ON -DCMAKE_BUILD_TYPE=Release -DIMPORT_DEPTH_CAM_FW=ON ../

make -j$(nproc)
sudo make install

#----------------------------------------------------
# (Optional) gphoto2
sudo apt-get install -y build-essential libltdl-dev libusb-1.0-0-dev libexif-dev udev libpopt-dev libudev-dev pkg-config git automake autoconf autopoint gettext libtool wget

git clone --branch libgphoto2-2_5_31-release --depth 1 https://github.com/gphoto/libgphoto2.git
cd libgphoto2
autoreconf --install --symlink
./configure
make -j$(nproc)
sudo make install

git clone --branch gphoto2-2_5_28-release --depth 1 https://github.com/gphoto/gphoto2.git
cd gphoto2
autoreconf --install --symlink
./configure
make -j$(nproc)
sudo make install

sudo ldconfig

udev_version=$(udevadm --version)
if   [ "$udev_version" -ge "201" ]
then
  udev_rules=201
elif [ "$udev_version" -ge "175" ]
then
  udev_rules=175
elif [ "$udev_version" -ge "136" ]
then
  udev_rules=136
else
  udev_rules=0.98
fi
set +H
sudo sh -c "/usr/local/lib/libgphoto2/print-camera-list udev-rules version $udev_rules group plugdev mode 0660 > /etc/udev/rules.d/90-libgphoto2.rules"
set -H
if   [ "$udev_rules" = "201" ]
then
set +H
sudo sh -c "/usr/local/lib/libgphoto2/print-camera-list hwdb > /etc/udev/hwdb.d/20-gphoto.hwdb"
set -H
fi

gphoto2 --version
gphoto2 --abilities
# gphoto2 --capture-image-and-download --filename /tmp/hoge.jpg

#----------------------------------------------------
# (Optional) OpenCV
# Refer to openCV_platform.sh

#----------------------------------------------------
# (Optional) TensorFlow Lite
sudo apt-get install cmake curl
git clone --branch v2.6.0 --depth 1 https://github.com/tensorflow/tensorflow.git
cd tensorflow
./tensorflow/lite/tools/make/download_dependencies.sh
# Raspberry pi Bullseye 64bit
#./tensorflow/lite/tools/make/build_aarch64_lib.sh
./tensorflow/lite/tools/make/build_lib.sh
# update flatbuffers
cd tensorflow/lite/tools/make/downloads
rm -rf flatbuffers
git clone -b v2.0.0 --depth=1 --recursive https://github.com/google/flatbuffers.git
cd flatbuffers
mkdir build
cd build
cmake ..
make -j4
sudo make install
sudo ldconfig

#----------------------------------------------------
# (Optional) Jetson inference
# CuDNN, download the latest .deb from NVIDIA site
# sudo dpkg -i cudnn-local-repo-ubuntu2204-8.8.0.121_1.0-1_amd64.deb

# TensorRT

# Jetson Nano/Xavier
sudo apt-get -y install libpython3-dev python3-numpy
git clone --recursive --depth 1 https://github.com/dusty-nv/jetson-inference.git
cd jetson-inference
git submodule update --init --recursive
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
make -j$(nproc)
sudo make install
sudo ldconfig

#----------------------------------------------------
# (Optional) Chilitags
git clone --depth 1 https://github.com/chili-epfl/chilitags.git
cd chilitags
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
make -j$(nproc)
sudo make install

#----------------------------------------------------
# (Optional) Open3D
# Refer to open3D_platform.sh

#----------------------------------------------------
# (Optional) gwsocket
git clone --depth 1 https://github.com/allinurl/gwsocket.git
cd gwsocket
autoreconf -fiv
./configure
make -j$(nproc)

#----------------------------------------------------
# (Optional) Vzense Nebular
git clone --depth 1 https://github.com/Vzense/NebulaSDK.git
# In direct connection mode configure the host ip to be 
# IP: 192.168.1.100
# Netmask: 255.255.255.0
# Gateway: 192.168.100.1

#----------------------------------------------------
# (Optional) Livox
git clone --depth 1 https://github.com/yankailab/Livox-SDK.git
cd Livox-SDK
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
make -j$(nproc)
sudo make install
# In direct connection mode configure the host ip to be 
# IP: 192.168.1.50
# Netmask: 255.255.255.0
# Gateway: 192.168.1.1

#----------------------------------------------------
# (Optional) Livox SDK2
git clone --depth 1 https://github.com/Livox-SDK/Livox-SDK2.git
cd Livox-SDK2
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
make -j$(nproc)
sudo make install
# In direct connection mode configure the host ip to be 
# IP: 192.168.1.50
# Netmask: 255.255.255.0
# Gateway: 192.168.1.1

#----------------------------------------------------
# (Optional) xArm
git clone --depth 1 https://github.com/xArm-Developer/xArm-CPLUS-SDK.git
cd xArm-CPLUS-SDK/
make all -j$(nproc)
sudo make install

#----------------------------------------------------
# (Optional) OpenKAI extra modules
git clone --depth 1 https://github.com/yankailab/OpenKAI_exm.git

#----------------------------------------------------
# OpenKAI
sudo apt-get install libunwind-dev
sudo apt-get install libgoogle-glog-dev
git clone --depth 1 https://github.com/yankailab/OpenKAI.git
cd OpenKAI
mkdir build
cd build
ccmake ../
make all -j$(nproc)

# To build with ROS2
echo "source /opt/ros/humble/setup.bash" >> ~/.bashrc
echo "source /home/kai/dev/rosWS/install/setup.bash" >> ~/.bashrc
#echo "source /home/kai/dev/ros2_humble/install/local_setup.bash" >> ~/.bashrc


#----------------------------------------------------
# (Optional) Pangolin & Orb_Slam3
git clone --branch v0.8 --depth 1 https://github.com/stevenlovegrove/Pangolin.git
cd Pangolin
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
make all -j$(nproc)
sudo make install

#git clone --depth 1 https://github.com/yankailab/ORB_SLAM3.git
git clone --branch v1.0-release --depth 1 https://github.com/UZ-SLAMLab/ORB_SLAM3.git
cd ORB_SLAM3
sed -i 's/++11/++14/g' CMakeLists.txt
chmod +x build.sh
./build.sh
# sudo make install

#----------------------------------------------------
# (Optional) Dynamixel
git clone --depth 1 https://github.com/ROBOTIS-GIT/DynamixelSDK.git
cd DynamixelSDK/c++/build/linux64
make -j$(nproc)
sudo make install
# For Jetson and Raspberry Pi
# remove "-m64" in "FORMAT" line of the Makefile in linux64 directory
