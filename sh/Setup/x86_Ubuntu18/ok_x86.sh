#!/bin/bash

# Update kernel
uname -r

wget https://kernel.ubuntu.com/~kernel-ppa/mainline/v5.2.11/linux-headers-5.2.11-050211_5.2.11-050211.201908290731_all.deb
wget https://kernel.ubuntu.com/~kernel-ppa/mainline/v5.2.11/linux-headers-5.2.11-050211-generic_5.2.11-050211.201908290731_amd64.deb
wget https://kernel.ubuntu.com/~kernel-ppa/mainline/v5.2.11/linux-image-unsigned-5.2.11-050211-generic_5.2.11-050211.201908290731_amd64.deb
wget https://kernel.ubuntu.com/~kernel-ppa/mainline/v5.2.11/linux-modules-5.2.11-050211-generic_5.2.11-050211.201908290731_amd64.deb
sudo dpkg -i *.deb
sudo reboot now

sudo apt-get update

# Dependencies
sudo apt-get -y install build-essential cmake cmake-curses-gui git autoconf automake libtool pkg-config libssl-dev libboost-all-dev libgflags-dev libgoogle-glog-dev uuid-dev libboost-filesystem-dev libboost-system-dev libboost-thread-dev ncurses-dev libprotobuf-dev protobuf-compiler libatlas-base-dev libopenblas-base libopenblas-dev liblapack-dev liblapack3 gsl-bin libgsl0-dev gstreamer1.0-0 gstreamer1.0-plugins-base libgstreamer1.0-0 libgstreamer-plugins-base1.0-dev gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-doc gstreamer1.0-tools gstreamer1.0-x gstreamer1.0-alsa gstreamer1.0-gl gstreamer1.0-gtk3 gstreamer1.0-qt5 gstreamer1.0-pulseaudio libdc1394-22 libdc1394-22-dev libjpeg-dev libpng-dev libtiff-dev libavcodec-dev libavformat-dev libswscale-dev libv4l-dev libtheora-dev libxvidcore-dev x264 v4l-utils libturbojpeg libvorbis-dev libfaac-dev libmp3lame-dev libopencore-amrnb-dev libopencore-amrwb-dev libusb-1.0-0-dev libusb-dev libudev-dev libglu1-mesa-dev libgl1-mesa-dev mesa-common-dev freeglut3-dev libglew-dev libgtk-3-dev libglfw3-dev libflann-dev libvtk6-dev libproj-dev

# Optional
sudo apt-get -y install apache2

sudo apt-get -y install libimage-exiftool-perl

sudo apt-get -y install python-pip
pip install --upgrade google-api-python-client google-auth-httplib2 google-auth-oauthlib

# (Optional) CUDA
wget http://developer.download.nvidia.com/compute/cuda/10.1/Prod/local_installers/cuda_10.1.243_418.87.00_linux.run
sudo sh cuda_10.1.243_418.87.00_linux.run
sudo echo -e "export PATH=/usr/local/cuda/bin:\$PATH\nexport LD_LIBRARY_PATH=/usr/local/cuda/lib64:\$LD_LIBRARY_PATH\nexport LC_ALL=en_US.UTF-8" >> ~/.bashrc

wget https://developer.download.nvidia.com/compute/machine-learning/repos/ubuntu1804/x86_64/libcudnn7_7.6.3.30-1+cuda10.1_amd64.deb
wget https://developer.download.nvidia.com/compute/machine-learning/repos/ubuntu1804/x86_64/libcudnn7-dev_7.6.3.30-1+cuda10.1_amd64.deb
sudo dpkg -i libcudnn7_7.6.3.30-1+cuda10.1_amd64.deb
sudo dpkg -i libcudnn7-dev_7.6.3.30-1+cuda10.1_amd64.deb

sudo apt autoremove
sudo apt clean

# CMake
wget https://github.com/Kitware/CMake/releases/download/v3.15.3/cmake-3.15.3.tar.gz
tar xvf cmake-3.15.3.tar.gz
rm -r cmake-3.15.3.tar.gz
cd cmake-3.15.3
./bootstrap
make -j12
sudo make install
sudo reboot now

# Eigen
wget --no-check-certificate http://bitbucket.org/eigen/eigen/get/3.3.7.tar.gz
tar xzvf 3.3.7.tar.gz
rm -r 3.3.7.tar.gz
cd eigen-eigen-323c052e1731
mkdir build
cd build
cmake ../
sudo make install

# (Optional) PCL
git clone https://github.com/PointCloudLibrary/pcl.git
cd pcl
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
make all -j12
sudo make install

# (Optional) MYNT EYE
git clone https://github.com/slightech/MYNT-EYE-D-SDK.git
cd MYNT-EYE-D-SDK
make init
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
make all -j12
sudo make install

# (Optional) RealSense
git clone https://github.com/IntelRealSense/librealsense.git
cd librealsense
./scripts/setup_udev_rules.sh
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
make -j8
sudo make install

# OpenCV
git clone --branch 4.1.1 https://github.com/opencv/opencv.git
git clone --branch 4.1.1 https://github.com/opencv/opencv_contrib.git

# OpenCV with CUDA

cd opencv
mkdir build
cd build
cmake -DBUILD_CUDA_STUBS=OFF -DBUILD_DOCS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_JASPER=OFF -DBUILD_JAVA=OFF -DBUILD_JPEG=OFF -DBUILD_OPENEXR=OFF -DBUILD_PACKAGE=ON -DBUILD_PERF_TESTS=OFF -DBUILD_PNG=OFF -DBUILD_PROTOBUF=ON -DBUILD_SHARED_LIBS=ON -DBUILD_TBB=OFF -DBUILD_TESTS=OFF -DBUILD_TIFF=OFF -DBUILD_WITH_DEBUG_INFO=OFF -DBUILD_WITH_DYNAMIC_IPP=OFF -DBUILD_ZLIB=OFF -DBUILD_opencv_apps=ON -DBUILD_opencv_aruco=ON -DBUILD_opencv_bgsegm=ON -DBUILD_opencv_bioinspired=ON -DBUILD_opencv_calib3d=ON -DBUILD_opencv_ccalib=ON -DBUILD_opencv_core=ON -DBUILD_opencv_cudaarithm=ON -DBUILD_opencv_cudabgsegm=ON -DBUILD_opencv_cudacodec=OFF -DBUILD_opencv_cudafeatures2d=ON -DBUILD_opencv_cudafilters=ON -DBUILD_opencv_cudaimgproc=ON -DBUILD_opencv_cudalegacy=OFF -DBUILD_opencv_cudaobjdetect=ON -DBUILD_opencv_cudaoptflow=ON -DBUILD_opencv_cudastereo=ON -DBUILD_opencv_cudawarping=ON -DBUILD_opencv_cudev=ON -DBUILD_opencv_datasets=ON -DBUILD_opencv_dnn=ON -DBUILD_opencv_dnn_objdetect=ON -DBUILD_opencv_dpm=ON -DBUILD_opencv_face=ON -DBUILD_opencv_features2d=ON -DBUILD_opencv_flann=ON -DBUILD_opencv_freetype=OFF -DBUILD_opencv_fuzzy=OFF -DBUILD_opencv_highgui=ON -DBUILD_opencv_img_hash=ON -DBUILD_opencv_imgcodecs=ON -DBUILD_opencv_imgproc=ON -DBUILD_opencv_java_bindings_generator=OFF -DBUILD_opencv_js=OFF -DBUILD_opencv_line_descriptor=ON -DBUILD_opencv_ml=ON -DBUILD_opencv_objdetect=ON -DBUILD_opencv_optflow=ON -DBUILD_opencv_phase_unwrapping=ON -DBUILD_opencv_photo=ON -DBUILD_opencv_plot=ON -DBUILD_opencv_python_bindings_generator=ON -DBUILD_opencv_python_tests=OFF -DBUILD_opencv_reg=ON -DBUILD_opencv_rgbd=ON -DBUILD_opencv_saliency=OFF -DBUILD_opencv_sfm=ON -DBUILD_opencv_shape=ON -DBUILD_opencv_stereo=ON -DBUILD_opencv_stitching=ON -DBUILD_opencv_structured_light=ON -DBUILD_opencv_superres=ON -DBUILD_opencv_surface_matching=ON -DBUILD_opencv_text=ON -DBUILD_opencv_tracking=ON -DBUILD_opencv_ts=ON -DBUILD_opencv_video=ON -DBUILD_opencv_videoio=ON -DBUILD_opencv_videostab=ON -DBUILD_opencv_world=OFF -DBUILD_opencv_xfeatures2d=ON -DBUILD_opencv_ximgproc=ON -DBUILD_opencv_xobjdetect=ON -DBUILD_opencv_xphoto=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_CONFIGURATION_TYPES=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DCUDA_ARCH_BIN=7.5 -DCUDA_ARCH_PTX="" -DCUDA_FAST_MATH=ON -DENABLE_BUILD_HARDENING=OFF -DENABLE_FAST_MATH=ON -DENABLE_PRECOMPILED_HEADERS=OFF -DOPENCV_ENABLE_NONFREE=OFF -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules -DWITH_1394=OFF -DWITH_ARAVIS=OFF -DWITH_CAROTENE=ON -DWITH_CLP=OFF -DWITH_CUBLAS=ON -DWITH_CUDA=ON -DWITH_CUFFT=ON -DWITH_EIGEN=ON -DWITH_FFMPEG=ON -DWITH_GDAL=OFF -DWITH_GDCM=OFF -DWITH_GIGEAPI=OFF -DWITH_GPHOTO2=ON -DWITH_GSTREAMER=ON -DWITH_GSTREAMER_0_10=OFF -DWITH_GTK=ON -DWITH_GTK_2_X=OFF -DWITH_HALIDE=OFF -DWITH_HPX=OFF -DWITH_INF_ENGINE=ON -DWITH_IPP=ON -DWITH_ITT=ON -DWITH_JASPER=ON -DWITH_JPEG=ON -DWITH_LAPACK=ON -DWITH_LIBREALSENSE=OFF -DWITH_MATLAB=OFF -DWITH_MFX=OFF -DWITH_NVCUVID=ON -DWITH_OPENCL=ON -DWITH_OPENCLAMDBLAS=ON -DWITH_OPENCLAMDFFT=ON -DWITH_OPENCL_SVM=OFF -DWITH_OPENEXR=ON -DWITH_OPENGL=ON -DWITH_OPENMP=OFF -DWITH_OPENNI=OFF -DWITH_OPENNI2=OFF -DWITH_OPENVX=OFF -DWITH_PNG=ON -DWITH_PROTOBUF=ON -DWITH_PTHREADS_PF=ON -DWITH_PVAPI=OFF -DWITH_QT=OFF -DWITH_TBB=ON -DWITH_TIFF=ON -DWITH_V4L=ON -DWITH_VA=OFF -DWITH_VA_INTEL=OFF -DWITH_VTK=ON -DWITH_WEBP=ON -DWITH_XIMEA=OFF -DWITH_XINE=OFF ../
make all -j8
sudo make install -j8

# OpenCV without CUDA
cd opencv
mkdir build
cd build
cmake -DBUILD_CUDA_STUBS=OFF -DBUILD_DOCS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_JASPER=OFF -DBUILD_JAVA=OFF -DBUILD_JPEG=OFF -DBUILD_OPENEXR=OFF -DBUILD_PACKAGE=ON -DBUILD_PERF_TESTS=OFF -DBUILD_PNG=OFF -DBUILD_PROTOBUF=ON -DBUILD_SHARED_LIBS=ON -DBUILD_TBB=OFF -DBUILD_TESTS=OFF -DBUILD_TIFF=OFF -DBUILD_WITH_DEBUG_INFO=OFF -DBUILD_WITH_DYNAMIC_IPP=OFF -DBUILD_ZLIB=OFF -DBUILD_opencv_apps=ON -DBUILD_opencv_aruco=ON -DBUILD_opencv_bgsegm=ON -DBUILD_opencv_bioinspired=ON -DBUILD_opencv_calib3d=ON -DBUILD_opencv_ccalib=ON -DBUILD_opencv_core=ON -DBUILD_opencv_cudev=ON -DBUILD_opencv_datasets=ON -DBUILD_opencv_dnn=ON -DBUILD_opencv_dnn_objdetect=ON -DBUILD_opencv_dpm=ON -DBUILD_opencv_face=ON -DBUILD_opencv_features2d=ON -DBUILD_opencv_flann=ON -DBUILD_opencv_freetype=OFF -DBUILD_opencv_fuzzy=OFF -DBUILD_opencv_highgui=ON -DBUILD_opencv_img_hash=ON -DBUILD_opencv_imgcodecs=ON -DBUILD_opencv_imgproc=ON -DBUILD_opencv_java_bindings_generator=OFF -DBUILD_opencv_js=OFF -DBUILD_opencv_line_descriptor=ON -DBUILD_opencv_ml=ON -DBUILD_opencv_objdetect=ON -DBUILD_opencv_optflow=ON -DBUILD_opencv_phase_unwrapping=ON -DBUILD_opencv_photo=ON -DBUILD_opencv_plot=ON -DBUILD_opencv_python_bindings_generator=ON -DBUILD_opencv_reg=ON -DBUILD_opencv_rgbd=ON -DBUILD_opencv_saliency=OFF -DBUILD_opencv_sfm=ON -DBUILD_opencv_shape=ON -DBUILD_opencv_stereo=ON -DBUILD_opencv_stitching=ON -DBUILD_opencv_structured_light=ON -DBUILD_opencv_superres=ON -DBUILD_opencv_surface_matching=ON -DBUILD_opencv_text=ON -DBUILD_opencv_tracking=ON -DBUILD_opencv_ts=ON -DBUILD_opencv_video=ON -DBUILD_opencv_videoio=ON -DBUILD_opencv_videostab=ON -DBUILD_opencv_world=OFF -DBUILD_opencv_xfeatures2d=ON -DBUILD_opencv_ximgproc=ON -DBUILD_opencv_xobjdetect=ON -DBUILD_opencv_xphoto=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_CONFIGURATION_TYPES=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DENABLE_BUILD_HARDENING=OFF -DENABLE_FAST_MATH=ON -DENABLE_PRECOMPILED_HEADERS=OFF -DOPENCV_ENABLE_NONFREE=OFF -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules -DWITH_1394=OFF -DWITH_ARAVIS=OFF -DWITH_CAROTENE=ON -DWITH_CLP=OFF -DWITH_CUDA=OFF -DWITH_EIGEN=ON -DWITH_FFMPEG=ON -DWITH_GDAL=OFF -DWITH_GDCM=OFF -DWITH_GIGEAPI=OFF -DWITH_GPHOTO2=ON -DWITH_GSTREAMER=ON -DWITH_GSTREAMER_0_10=OFF -DWITH_GTK=ON -DWITH_GTK_2_X=OFF -DWITH_HALIDE=OFF -DWITH_HPX=OFF -DWITH_INF_ENGINE=ON -DWITH_IPP=ON -DWITH_ITT=ON -DWITH_JASPER=ON -DWITH_JPEG=ON -DWITH_LAPACK=ON -DWITH_LIBREALSENSE=OFF -DWITH_MATLAB=OFF -DWITH_MFX=OFF -DWITH_OPENCL=ON -DWITH_OPENCLAMDBLAS=ON -DWITH_OPENCLAMDFFT=ON -DWITH_OPENCL_SVM=OFF -DWITH_OPENEXR=ON -DWITH_OPENGL=ON -DWITH_OPENMP=OFF -DWITH_OPENNI=OFF -DWITH_OPENNI2=OFF -DWITH_OPENVX=OFF -DWITH_PNG=ON -DWITH_PROTOBUF=ON -DWITH_PTHREADS_PF=ON -DWITH_PVAPI=OFF -DWITH_QT=OFF -DWITH_TBB=ON -DWITH_TIFF=ON -DWITH_V4L=ON -DWITH_VA=OFF -DWITH_VA_INTEL=OFF -DWITH_VTK=ON -DWITH_WEBP=ON -DWITH_XIMEA=OFF -DWITH_XINE=OFF ../
make all -j8
sudo make install -j8

# gwsocket
git clone https://github.com/allinurl/gwsocket.git
cd gwsocket
autoreconf -fiv
./configure
make -j8

# (Optional) Tesseract
sudo apt-get -y install libleptonica-dev libcurl4-openssl-dev liblog4cplus-dev libpng-dev libjpeg8-dev libtiff5-dev zlib1g-dev
git clone https://github.com/tesseract-ocr/tesseract.git
cd tesseract
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
make all -j8
sudo make install

# (Optional) OpenALPR
git clone https://github.com/yankailab/openalpr.git
cd openalpr
cd src
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
make all -j8
sudo make install
sudo ldconfig

# (Optional) Dynamixel
git clone https://github.com/ROBOTIS-GIT/DynamixelSDK.git
cd DynamixelSDK/c++/build/linux64
make
sudo make install

# OpenKAI
git clone https://github.com/yankailab/OpenKAI.git
cd OpenKAI
mkdir build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DUSE_CUDA=ON -DCUDA_ARCH=62 -DUSE_OPENCV_CONTRIB=ON -DUSE_DARKNET=ON -DDarknet_root=/home/ubuntu/dev/darknet -DUSE_REALSENSE=OFF -Dlibrealsense_root=/home/ubuntu/dev/librealsense -DUSE_TENSORRT=ON -DTensorRT_build=/home/ubuntu/dev/jetson-inference-batch/build/aarch64 ../
make all -j8

# Copy startup sh into home
sudo chmod a+x $HOME/ok.sh


# Outdated, to be updated

# OpenTracker
git clone https://github.com/rockkingjy/OpenTracker
cd OpenTracker/eco
#edit makefile -> USE_SIMD=2
make -j8

# Darknet
cd $FULLDEVDIR
git clone https://github.com/yankailab/darknet.git
cd darknet
make -j6
cd data
wget --no-check-certificate https://pjreddie.com/media/files/yolov2.weights
wget --no-check-certificate https://pjreddie.com/media/files/yolov3.weights
wget --no-check-certificate https://pjreddie.com/media/files/yolov3-tiny.weights
wget --no-check-certificate https://pjreddie.com/media/files/yolov3-spp.weights

# Pangolin
cd $FULLDEVDIR
git clone https://github.com/yankailab/Pangolin.git
cd Pangolin
mkdir build
cd build
cmake ../
cmake --build .
sudo make install

# ORB_SLAM2_GPU
cd $FULLDEVDIR
git clone https://github.com/yankailab/orb_slam2_gpu.git
cd orb_slam2_gpu
chmod +x build.sh
./build.sh

# Jetson-inference-batch
cd $FULLDEVDIR
git clone https://github.com/yankailab/jetson-inference-batch.git
cd jetson-inference-batch
cp CMakeLists_tegra.txt CMakeLists.txt
mkdir build
cd build
cmake ../
make
