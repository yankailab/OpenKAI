#!/bin/bash
# License: Free

# Change performace setting and make it auto start
sudo rm /etc/rc.local
set +H
sudo sh -c "echo '#!/bin/sh\n/home/ubuntu/jetson_clocks.sh\nnvpmodel -m 0\nexit 0\n' >> /etc/rc.local"
set -H
sudo chmod a+x /etc/rc.local
sudo chmod a+x /home/ubuntu/jetson_clocks.sh

# Install Prerequisites
#Base
sudo apt-get update
sudo apt-get -y install build-essential cmake cmake-curses-gui git libboost-all-dev libgflags-dev libgoogle-glog-dev uuid-dev libboost-filesystem-dev libboost-system-dev libboost-thread-dev ncurses-dev

#BLAS
sudo apt-get -y install libatlas-base-dev libopenblas-base libopenblas-dev liblapack-dev liblapack3

#Codecs
sudo apt-get -y install libdc1394-22 libdc1394-22-dev libjpeg-dev libpng12-dev libpng-dev libtiff-dev libjasper-dev libavcodec-dev libavformat-dev libswscale-dev libv4l-dev libtheora-dev libxvidcore-dev x264 v4l-utils gstreamer1.0 gstreamer1.0-tools gstreamer1.0-plugins-ugly libturbojpeg libvorbis-dev libfaac-dev libmp3lame-dev libopencore-amrnb-dev libopencore-amrwb-dev

#IO
sudo apt-get -y install libusb-1.0-0-dev libudev-dev

#GUI
sudo apt-get -y install libgtk2.0-dev libglew-dev

# Add CUDA to bash (JetPack 3.2 DP only)
sudo echo -e "export PATH=/usr/local/cuda/bin:\$PATH\nexport LD_LIBRARY_PATH=/usr/local/cuda/lib64:\$LD_LIBRARY_PATH" >> ~/.bashrc
bash

# Put everything into dev
cd $HOME
mkdir dev

# Update Eigen
cd $HOME/dev
wget --no-check-certificate http://bitbucket.org/eigen/eigen/get/3.3.4.tar.gz
tar xzvf 3.3.4.tar.gz
sudo rm -r 3.3.4.tar.gz
cd eigen-eigen-5a0156e40feb
mkdir build
cd build
cmake ../
sudo make install

# Install OpenCV
cd $HOME/dev
git clone --branch 3.4.0 https://github.com/opencv/opencv.git
git clone --branch 3.4.0 https://github.com/opencv/opencv_contrib.git
cd $HOME/opencv
mkdir build
cd build 
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_PNG=OFF -DBUILD_TIFF=OFF -DBUILD_TBB=OFF -DBUILD_JPEG=OFF -DBUILD_JASPER=OFF -DBUILD_ZLIB=OFF -DBUILD_EXAMPLES=OFF -DBUILD_TESTS=OFF -DBUILD_opencv_java=OFF -DBUILD_opencv_python2=ON -DBUILD_opencv_python3=OFF -DENABLE_PRECOMPILED_HEADERS=OFF -DWITH_OPENCL=OFF -DWITH_OPENMP=OFF -DWITH_FFMPEG=ON -DWITH_GSTREAMER=ON -DCUDA_FAST_MATH=ON -DWITH_V4L=ON -DWITH_GSTREAMER_0_10=OFF -DWITH_CUDA=ON -DWITH_GTK=ON -DWITH_VTK=OFF -DWITH_TBB=ON -DWITH_1394=OFF -DWITH_OPENEXR=OFF -DCUDA_TOOLKIT_ROOT_DIR=/usr/local/cuda-8.0 -DCUDA_ARCH_BIN=5.3 -DCUDA_ARCH_PTX="" -DINSTALL_C_EXAMPLES=OFF -DINSTALL_TESTS=OFF -DOPENCV_EXTRA_MODULES_PATH=../opencv_contrib/modules ../
make -j4
sudo make install

# Pangolin
cd $HOME/dev
git clone https://github.com/yankailab/Pangolin.git
cd Pangolin
mkdir build
cd build
cmake ../
cmake --build .
sudo make install

# Orb_SLAM2 GPU VERSION
cd $HOME/dev
git clone https://github.com/yankailab/orb_slam2_gpu.git
cd orb_slam2_gpu
chmod +x build.sh
./build.sh

# Jetson-inference-batch
cd $HOME/dev
git clone https://github.com/yankailab/jetson-inference-batch.git
cd jetson-inference-batch/
cp CMakeLists_tegra.txt CMakeLists.txt
mkdir build
cd build
cmake ../
make

# Darknet
cd $HOME/dev
git clone https://github.com/yankailab/darknet.git
cd darknet
make
mkdir model
cd model
wget --no-check-certificate https://pjreddie.com/media/files/yolo.weights

#Build OpenKAI
cd $HOME/dev
git clone https://github.com/yankailab/OpenKAI.git
cd OpenKAI
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DCUDA_ARCH=62 -DUSE_OPENCV2X=OFF -DUSE_OPENCV_CONTRIB=ON -DTensorRT_build=/home/nvidia/jetson-inference/build/aarch64 ../
make all -j4

