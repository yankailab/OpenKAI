#Installation on Ubuntu 16.04LTS

##prerequisites
  ```Shell
sudo apt-get update
sudo apt-get -y install git cmake build-essential cmake-curses-gui libatlas-base-dev libprotobuf-dev libleveldb-dev libsnappy-dev libboost-all-dev libhdf5-serial-dev libgflags-dev libgoogle-glog-dev liblmdb-dev protobuf-compiler libgtk2.0-dev pkg-config python-dev python-numpy libdc1394-22 libdc1394-22-dev libjpeg-dev libpng12-dev libjasper-dev libavcodec-dev libavformat-dev libswscale-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev libv4l-dev libtbb-dev libqt4-dev libfaac-dev libmp3lame-dev libopencore-amrnb-dev libopencore-amrwb-dev libtheora-dev libvorbis-dev libxvidcore-dev x264 v4l-utils unzip python-protobuf python-scipy python-pip libeigen3-dev default-jre
  ```

#cuda
  ```Shell
./cuda_8.0.44_linux.run
sudo echo -e "export PATH=/usr/local/cuda/bin:\$PATH\nexport LD_LIBRARY_PATH=/usr/local/cuda/lib64:\$LD_LIBRARY_PATH" >> ~/.bashrc
  ```

#CuDNN
  ```Shell
tar xzvf cudnn-8.0-linux-x64-v5.1.tgz
cd cuda
sudo cp lib64/lib* /usr/local/cuda/lib64/
sudo cp include/* /usr/local/cuda/include/
sudo ldconfig
  ```

#git repo
  ```Shell
cd src
git clone https://github.com/Itseez/opencv.git
git clone https://github.com/Itseez/opencv_contrib.git
git clone https://github.com/yankailab/jetson-inference.git
git clone https://github.com/yankailab/OpenKAI.git
  ```

#OpenCV
opencv/modules/core/include/opencv2/core/version.hpp
#define CV_VERSION_MAJOR    3
#define CV_VERSION_MINOR    1
#define CV_VERSION_REVISION 0
  ```Shell
  ```

#ZED
./ZED_SDK_Linux

#TensorRT (GIE)
  ```Shell
sudo cp -rp libgie1/usr/* /usr/
sudo cp -rp libgie-dev/usr/* /usr/
  ```

#jetson-inference

find_package(CUDA)

set(
	CUDA_NVCC_FLAGS
	${CUDA_NVCC_FLAGS}; 
    -O3 -gencode arch=compute_61,code=sm_61
)

  ```Shell
include_directories(/usr/include/gstreamer-1.0 /usr/lib/aarch64-linux-gnu/gstreamer-1.0/include /usr/include/glib-2.0 /usr/include/libxml2 /usr/lib/aarch64-linux-gnu/glib-2.0/include/)
  ```
  ```Shell
include_directories(/usr/include/gstreamer-1.0 /usr/lib/x86_64-linux-gnu/gstreamer-1.0/include /usr/include/glib-2.0 /usr/include/libxml2 /usr/lib/x86_64-linux-gnu/glib-2.0/include/)
  ```


