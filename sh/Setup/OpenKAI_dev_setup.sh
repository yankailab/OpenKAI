#!/bin/bash

#----------------------------------------------------
# (Optional) Update kernel if needed
uname -r
wget https://kernel.ubuntu.com/~kernel-ppa/mainline/v5.12/amd64/linux-headers-5.12.0-051200_5.12.0-051200.202104252130_all.deb
wget https://kernel.ubuntu.com/~kernel-ppa/mainline/v5.12/amd64/linux-headers-5.12.0-051200-generic_5.12.0-051200.202104252130_amd64.deb
wget https://kernel.ubuntu.com/~kernel-ppa/mainline/v5.12/amd64/linux-image-unsigned-5.12.0-051200-generic_5.12.0-051200.202104252130_amd64.deb
wget https://kernel.ubuntu.com/~kernel-ppa/mainline/v5.12/amd64/linux-modules-5.12.0-051200-generic_5.12.0-051200.202104252130_amd64.deb
sudo dpkg -i *.deb
sudo reboot now

#----------------------------------------------------
# (Optional) sshd
sudo apt-get install ssh
systemctl start sshd

#----------------------------------------------------
# System setup
sudo apt-get update
#sudo apt-get upgrade
#sudo apt-get dist-upgrade

# GCC
sudo apt-get -y install g++-10 gcc-10
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-10 10 --slave /usr/bin/g++ g++ /usr/bin/g++-10
sudo update-alternatives --config gcc

# Basic
sudo apt-get -y install build-essential clang libc++-dev libc++abi-dev cmake cmake-curses-gui ninja-build git autoconf automake libtool pkg-config libssl-dev libboost-all-dev libgflags-dev libgoogle-glog-dev uuid-dev libboost-filesystem-dev libboost-system-dev libboost-thread-dev ncurses-dev libssl-dev libprotobuf-dev protobuf-compiler libcurl4 curl libusb-1.0-0-dev libusb-dev libudev-dev libc++-dev libc++abi-dev libfmt-dev

# Image, codecs, gstreamer
sudo apt-get -y install gstreamer1.0-0 gstreamer1.0-plugins-base libgstreamer1.0-0 libgstreamer-plugins-base1.0-dev gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-alsa gstreamer1.0-gl libv4l-dev v4l-utils libjpeg-dev libpng-dev libtiff-dev libavcodec-dev libavformat-dev libxvidcore-dev x264

# OpenGL
sudo apt-get -y install libglu1-mesa-dev libglu1-mesa libgl1-mesa-glx libgl1-mesa-dev libglfw3 libglfw3-dev libglew-dev mesa-common-dev freeglut3-dev xorg-dev libxt-dev libxi-dev libx11-dev

# Desktop
sudo apt-get -y install libgtk-3-dev gstreamer1.0-x gstreamer1.0-gtk3

# Numerical
sudo apt-get -y install libqhull-dev qhull-bin gfortran libblas-dev liblapack-dev liblapacke-dev liblapack3 libatlas-base-dev libopenblas-base libopenblas-dev gsl-bin libgsl0-dev libflann-dev libproj-dev

# Graph plotting
sudo apt-get -y install libmgl-dev

# Not sure if needed
sudo apt-get -y install libpcap-dev libdc1394-22 libdc1394-22-dev libswscale-dev libtheora-dev libvorbis-dev libfaac-dev libmp3lame-dev libopencore-amrnb-dev libopencore-amrwb-dev

# (Optional)
sudo apt-get -y install libimage-exiftool-perl
sudo apt-get -y install python python-pip
pip install --upgrade google-api-python-client google-auth-httplib2 google-auth-oauthlib
snap install cloudcompare

sudo apt autoremove
sudo apt clean

#----------------------------------------------------
# (Optional) CUDA
# Update the video driver first with Software and Update
sudo apt -y install gcc-8 g++-8
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 8 --slave /usr/bin/g++ g++ /usr/bin/g++-8
# Select gcc 8 for CUDA compatibility
sudo update-alternatives --config gcc
wget http://developer.download.nvidia.com/compute/cuda/10.2/Prod/local_installers/cuda_10.2.89_440.33.01_linux.run
sudo sh cuda_10.2.89_440.33.01_linux.run
sudo echo -e "export PATH=/usr/local/cuda/bin:\$PATH\nexport LD_LIBRARY_PATH=/usr/local/cuda/lib64:\$LD_LIBRARY_PATH\nexport LC_ALL=en_US.UTF-8" >> ~/.bashrc
# Switch back gcc to auto mode
sudo update-alternatives --config gcc
# CuDNN, download the latest .deb from NVIDIA site
sudo dpkg -i libcudnn7_7.6.5.32-1+cuda10.2_amd64.deb
sudo dpkg -i libcudnn7-dev_7.6.5.32-1+cuda10.2_amd64.deb

# CUDA 11
wget https://developer.download.nvidia.com/compute/cuda/11.2.2/local_installers/cuda_11.2.2_460.32.03_linux.run
sudo sh cuda_11.2.2_460.32.03_linux.run
sudo echo -e "export PATH=/usr/local/cuda/bin:\$PATH\nexport LD_LIBRARY_PATH=/usr/local/cuda/lib64:\$LD_LIBRARY_PATH\nexport LC_ALL=en_US.UTF-8" >> ~/.bashrc

#----------------------------------------------------
# CMake
wget https://github.com/Kitware/CMake/releases/download/v3.23.1/cmake-3.23.1.tar.gz
tar xvf cmake-3.23.1.tar.gz
cd cmake-3.23.1
./bootstrap
make -j$(nproc)
sudo make install
sudo reboot now

#----------------------------------------------------
# Eigen
git clone --depth 1 https://gitlab.com/libeigen/eigen.git
cd eigen
mkdir build
cd build
cmake ../
sudo make install

#----------------------------------------------------
# (Optional) RealSense
git clone --depth 1 https://github.com/IntelRealSense/librealsense.git
cd librealsense
./scripts/setup_udev_rules.sh
mkdir build
cd build
#cmake -DCMAKE_BUILD_TYPE=Release ../
cmake -DFORCE_LIBUVC=true -DBUILD_WITH_CUDA=ON -DCMAKE_BUILD_TYPE=Release -DIMPORT_DEPTH_CAM_FW=ON ../
make -j$(nproc)
sudo make install

#----------------------------------------------------
# (Optional) gphoto2
sudo apt-get install -y build-essential libltdl-dev libusb-1.0-0-dev libexif-dev udev libpopt-dev libudev-dev pkg-config git automake autoconf autopoint gettext libtool wget

git clone --branch libgphoto2-2_5_28-release --depth 1 https://github.com/gphoto/libgphoto2.git
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

# (Optional) v4l2loopback
git clone https://github.com/umlaeute/v4l2loopback.git
cd v4l2loopback
make -j$(nproc)
sudo make install
sudo depmod -a
sudo modprobe v4l2loopback
gphoto2 --stdout --capture-movie | ffmpeg -i - -vcodec rawvideo -pix_fmt yuv420p -threads 0 -f v4l2 /dev/video0

#----------------------------------------------------
# (Optional) OpenCV
git clone --branch 4.5.5 --depth 1 https://github.com/opencv/opencv.git
git clone --branch 4.5.5 --depth 1 https://github.com/opencv/opencv_contrib.git
cd opencv
mkdir build
cd build

# OpenCV with CUDA
cmake -DBUILD_CUDA_STUBS=OFF -DBUILD_DOCS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_JASPER=OFF -DBUILD_JAVA=OFF -DBUILD_JPEG=OFF -DBUILD_OPENEXR=OFF -DBUILD_PACKAGE=ON -DBUILD_PERF_TESTS=OFF -DBUILD_PNG=OFF -DBUILD_PROTOBUF=ON -DBUILD_SHARED_LIBS=ON -DBUILD_TBB=OFF -DBUILD_TESTS=OFF -DBUILD_TIFF=OFF -DBUILD_WITH_DEBUG_INFO=OFF -DBUILD_WITH_DYNAMIC_IPP=OFF -DBUILD_ZLIB=OFF -DBUILD_opencv_apps=ON -DBUILD_opencv_aruco=ON -DBUILD_opencv_bgsegm=ON -DBUILD_opencv_bioinspired=ON -DBUILD_opencv_calib3d=ON -DBUILD_opencv_ccalib=ON -DBUILD_opencv_core=ON -DBUILD_opencv_cudaarithm=ON -DBUILD_opencv_cudabgsegm=ON -DBUILD_opencv_cudacodec=OFF -DBUILD_opencv_cudafeatures2d=ON -DBUILD_opencv_cudafilters=ON -DBUILD_opencv_cudaimgproc=ON -DBUILD_opencv_cudalegacy=OFF -DBUILD_opencv_cudaobjdetect=ON -DBUILD_opencv_cudaoptflow=ON -DBUILD_opencv_cudastereo=ON -DBUILD_opencv_cudawarping=ON -DBUILD_opencv_cudev=ON -DBUILD_opencv_datasets=ON -DBUILD_opencv_dnn=ON -DBUILD_opencv_dnn_objdetect=ON -DBUILD_opencv_dpm=ON -DBUILD_opencv_face=ON -DBUILD_opencv_features2d=ON -DBUILD_opencv_flann=ON -DBUILD_opencv_freetype=ON -DBUILD_opencv_fuzzy=OFF -DBUILD_opencv_highgui=ON -DBUILD_opencv_img_hash=ON -DBUILD_opencv_imgcodecs=ON -DBUILD_opencv_imgproc=ON -DBUILD_opencv_java_bindings_generator=OFF -DBUILD_opencv_js=OFF -DBUILD_opencv_js_bindings_generator=OFF -DBUILD_opencv_line_descriptor=ON -DBUILD_opencv_ml=ON -DBUILD_opencv_objdetect=ON -DBUILD_opencv_optflow=ON -DBUILD_opencv_phase_unwrapping=ON -DBUILD_opencv_photo=ON -DBUILD_opencv_plot=ON -DBUILD_opencv_python_bindings_generator=ON -DBUILD_opencv_python_tests=OFF -DBUILD_opencv_reg=ON -DBUILD_opencv_rgbd=ON -DBUILD_opencv_saliency=OFF -DBUILD_opencv_sfm=ON -DBUILD_opencv_shape=ON -DBUILD_opencv_stereo=ON -DBUILD_opencv_stitching=ON -DBUILD_opencv_structured_light=ON -DBUILD_opencv_superres=ON -DBUILD_opencv_surface_matching=ON -DBUILD_opencv_text=ON -DBUILD_opencv_tracking=ON -DBUILD_opencv_ts=ON -DBUILD_opencv_video=ON -DBUILD_opencv_videoio=ON -DBUILD_opencv_videostab=ON -DBUILD_opencv_world=OFF -DBUILD_opencv_xfeatures2d=OFF -DBUILD_opencv_ximgproc=ON -DBUILD_opencv_xobjdetect=ON -DBUILD_opencv_xphoto=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_CONFIGURATION_TYPES=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DCUDA_ARCH_BIN=7.5 -DCUDA_ARCH_PTX="" -DCUDA_FAST_MATH=ON -DENABLE_BUILD_HARDENING=OFF -DENABLE_FAST_MATH=ON -DENABLE_PRECOMPILED_HEADERS=OFF -DOPENCV_ENABLE_NONFREE=OFF -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules -DWITH_1394=OFF -DWITH_ARAVIS=OFF -DWITH_CAROTENE=ON -DWITH_CLP=OFF -DWITH_CUBLAS=ON -DWITH_CUDA=ON -DWITH_CUFFT=ON -DWITH_EIGEN=ON -DWITH_FFMPEG=ON -DWITH_GDAL=OFF -DWITH_GDCM=OFF -DWITH_GIGEAPI=OFF -DWITH_GPHOTO2=ON -DWITH_GSTREAMER=ON -DWITH_GSTREAMER_0_10=OFF -DWITH_GTK=ON -DWITH_GTK_2_X=OFF -DWITH_HALIDE=OFF -DWITH_HPX=OFF -DWITH_INF_ENGINE=ON -DWITH_IPP=ON -DWITH_ITT=ON -DWITH_JASPER=ON -DWITH_JPEG=ON -DWITH_LAPACK=ON -DWITH_LIBREALSENSE=OFF -DWITH_MATLAB=OFF -DWITH_MFX=OFF -DWITH_NVCUVID=ON -DWITH_OPENCL=ON -DWITH_OPENCLAMDBLAS=ON -DWITH_OPENCLAMDFFT=ON -DWITH_OPENCL_SVM=OFF -DWITH_OPENEXR=ON -DWITH_OPENGL=ON -DWITH_OPENMP=OFF -DWITH_OPENNI=OFF -DWITH_OPENNI2=OFF -DWITH_OPENVX=OFF -DWITH_PNG=ON -DWITH_PROTOBUF=ON -DWITH_PTHREADS_PF=ON -DWITH_PVAPI=OFF -DWITH_QT=OFF -DWITH_TBB=ON -DWITH_TIFF=ON -DWITH_V4L=ON -DWITH_VA=OFF -DWITH_VA_INTEL=OFF -DWITH_VTK=ON -DWITH_WEBP=ON -DWITH_XIMEA=OFF -DWITH_XINE=OFF -DBUILD_opencv_hdf=OFF ../

# OpenCV without CUDA
cmake -DBUILD_CUDA_STUBS=OFF -DBUILD_DOCS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_JASPER=OFF -DBUILD_JAVA=OFF -DBUILD_JPEG=OFF -DBUILD_OPENEXR=OFF -DBUILD_PACKAGE=ON -DBUILD_PERF_TESTS=OFF -DBUILD_PNG=OFF -DBUILD_PROTOBUF=ON -DBUILD_SHARED_LIBS=ON -DBUILD_TBB=OFF -DBUILD_TESTS=OFF -DBUILD_TIFF=OFF -DBUILD_WITH_DEBUG_INFO=OFF -DBUILD_WITH_DYNAMIC_IPP=OFF -DBUILD_ZLIB=OFF -DBUILD_opencv_apps=ON -DBUILD_opencv_aruco=ON -DBUILD_opencv_bgsegm=ON -DBUILD_opencv_bioinspired=ON -DBUILD_opencv_calib3d=ON -DBUILD_opencv_ccalib=ON -DBUILD_opencv_core=ON -DBUILD_opencv_cudev=ON -DBUILD_opencv_datasets=ON -DBUILD_opencv_dnn=ON -DBUILD_opencv_dnn_objdetect=ON -DBUILD_opencv_dpm=ON -DBUILD_opencv_face=ON -DBUILD_opencv_features2d=ON -DBUILD_opencv_flann=ON -DBUILD_opencv_freetype=ON -DBUILD_opencv_fuzzy=OFF -DBUILD_opencv_highgui=ON -DBUILD_opencv_img_hash=ON -DBUILD_opencv_imgcodecs=ON -DBUILD_opencv_imgproc=ON -DBUILD_opencv_java_bindings_generator=OFF -DBUILD_opencv_js=OFF -DBUILD_opencv_js_bindings_generator=OFF -DBUILD_opencv_line_descriptor=ON -DBUILD_opencv_ml=ON -DBUILD_opencv_objdetect=ON -DBUILD_opencv_optflow=ON -DBUILD_opencv_phase_unwrapping=ON -DBUILD_opencv_photo=ON -DBUILD_opencv_plot=ON -DBUILD_opencv_python_bindings_generator=ON -DBUILD_opencv_reg=ON -DBUILD_opencv_rgbd=ON -DBUILD_opencv_saliency=OFF -DBUILD_opencv_sfm=ON -DBUILD_opencv_shape=ON -DBUILD_opencv_stereo=ON -DBUILD_opencv_stitching=ON -DBUILD_opencv_structured_light=ON -DBUILD_opencv_superres=ON -DBUILD_opencv_surface_matching=ON -DBUILD_opencv_text=ON -DBUILD_opencv_tracking=ON -DBUILD_opencv_ts=ON -DBUILD_opencv_video=ON -DBUILD_opencv_videoio=ON -DBUILD_opencv_videostab=ON -DBUILD_opencv_world=OFF -DBUILD_opencv_xfeatures2d=OFF -DBUILD_opencv_ximgproc=ON -DBUILD_opencv_xobjdetect=ON -DBUILD_opencv_xphoto=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_CONFIGURATION_TYPES=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DENABLE_BUILD_HARDENING=OFF -DENABLE_FAST_MATH=ON -DENABLE_PRECOMPILED_HEADERS=OFF -DOPENCV_ENABLE_NONFREE=OFF -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules -DWITH_1394=OFF -DWITH_ARAVIS=OFF -DWITH_CAROTENE=ON -DWITH_CLP=OFF -DWITH_CUDA=OFF -DWITH_EIGEN=ON -DWITH_FFMPEG=ON -DWITH_GDAL=OFF -DWITH_GDCM=OFF -DWITH_GIGEAPI=OFF -DWITH_GPHOTO2=ON -DWITH_GSTREAMER=ON -DWITH_GSTREAMER_0_10=OFF -DWITH_GTK=ON -DWITH_GTK_2_X=OFF -DWITH_HALIDE=OFF -DWITH_HPX=OFF -DWITH_INF_ENGINE=ON -DWITH_IPP=ON -DWITH_ITT=ON -DWITH_JASPER=ON -DWITH_JPEG=ON -DWITH_LAPACK=ON -DWITH_LIBREALSENSE=OFF -DWITH_MATLAB=OFF -DWITH_MFX=OFF -DWITH_OPENCL=ON -DWITH_OPENCLAMDBLAS=ON -DWITH_OPENCLAMDFFT=ON -DWITH_OPENCL_SVM=OFF -DWITH_OPENEXR=ON -DWITH_OPENGL=ON -DWITH_OPENMP=OFF -DWITH_OPENNI=OFF -DWITH_OPENNI2=OFF -DWITH_OPENVX=OFF -DWITH_PNG=ON -DWITH_PROTOBUF=ON -DWITH_PTHREADS_PF=ON -DWITH_PVAPI=OFF -DWITH_QT=OFF -DWITH_TBB=ON -DWITH_TIFF=ON -DWITH_V4L=ON -DWITH_VA=OFF -DWITH_VA_INTEL=OFF -DWITH_VTK=ON -DWITH_WEBP=ON -DWITH_XIMEA=OFF -DWITH_XINE=OFF -DBUILD_opencv_hdf=OFF ../

#Jetson Nano
cmake -DBUILD_CUDA_STUBS=OFF -DBUILD_DOCS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_JASPER=OFF -DBUILD_JAVA=OFF -DBUILD_JPEG=OFF -DBUILD_OPENEXR=OFF -DBUILD_PACKAGE=ON -DBUILD_PERF_TESTS=OFF -DBUILD_PNG=OFF -DBUILD_PROTOBUF=ON -DBUILD_SHARED_LIBS=ON -DBUILD_TBB=OFF -DBUILD_TESTS=OFF -DBUILD_TIFF=OFF -DBUILD_WITH_DEBUG_INFO=OFF -DBUILD_WITH_DYNAMIC_IPP=OFF -DBUILD_ZLIB=OFF -DBUILD_opencv_apps=ON -DBUILD_opencv_aruco=ON -DBUILD_opencv_bgsegm=ON -DBUILD_opencv_bioinspired=ON -DBUILD_opencv_calib3d=ON -DBUILD_opencv_ccalib=ON -DBUILD_opencv_core=ON -DBUILD_opencv_cudaarithm=ON -DBUILD_opencv_cudabgsegm=ON -DBUILD_opencv_cudacodec=ON -DBUILD_opencv_cudafeatures2d=ON -DBUILD_opencv_cudafilters=ON -DBUILD_opencv_cudaimgproc=ON -DBUILD_opencv_cudalegacy=OFF -DBUILD_opencv_cudaobjdetect=ON -DBUILD_opencv_cudaoptflow=ON -DBUILD_opencv_cudastereo=ON -DBUILD_opencv_cudawarping=ON -DBUILD_opencv_cudev=ON -DBUILD_opencv_datasets=ON -DBUILD_opencv_dnn=ON -DBUILD_opencv_dnn_objdetect=ON -DBUILD_opencv_dpm=ON -DBUILD_opencv_face=ON -DBUILD_opencv_features2d=ON -DBUILD_opencv_flann=ON -DBUILD_opencv_freetype=ON -DBUILD_opencv_fuzzy=OFF -DBUILD_opencv_highgui=ON -DBUILD_opencv_img_hash=ON -DBUILD_opencv_imgcodecs=ON -DBUILD_opencv_imgproc=ON -DBUILD_opencv_java_bindings_generator=OFF -DBUILD_opencv_js=OFF -DBUILD_opencv_js_bindings_generator=OFF -DBUILD_opencv_line_descriptor=ON -DBUILD_opencv_ml=ON -DBUILD_opencv_objdetect=ON -DBUILD_opencv_optflow=ON -DBUILD_opencv_phase_unwrapping=ON -DBUILD_opencv_photo=ON -DBUILD_opencv_plot=ON -DBUILD_opencv_python_bindings_generator=ON -DBUILD_opencv_python_tests=OFF -DBUILD_opencv_reg=ON -DBUILD_opencv_rgbd=ON -DBUILD_opencv_saliency=OFF -DBUILD_opencv_sfm=ON -DBUILD_opencv_shape=ON -DBUILD_opencv_stereo=ON -DBUILD_opencv_stitching=ON -DBUILD_opencv_structured_light=ON -DBUILD_opencv_superres=ON -DBUILD_opencv_surface_matching=ON -DBUILD_opencv_text=ON -DBUILD_opencv_tracking=ON -DBUILD_opencv_ts=ON -DBUILD_opencv_video=ON -DBUILD_opencv_videoio=ON -DBUILD_opencv_videostab=ON -DBUILD_opencv_world=OFF -DBUILD_opencv_xfeatures2d=OFF -DBUILD_opencv_ximgproc=ON -DBUILD_opencv_xobjdetect=ON -DBUILD_opencv_xphoto=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_CONFIGURATION_TYPES=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DCUDA_ARCH_BIN=5.3 -DCUDA_ARCH_PTX="" -DCUDA_FAST_MATH=ON -DENABLE_BUILD_HARDENING=OFF -DENABLE_FAST_MATH=ON -DENABLE_PRECOMPILED_HEADERS=OFF -DOPENCV_ENABLE_NONFREE=OFF -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules -DWITH_1394=OFF -DWITH_ARAVIS=OFF -DWITH_CAROTENE=ON -DWITH_CLP=OFF -DWITH_CUBLAS=ON -DWITH_CUDA=ON -DWITH_CUFFT=ON -DWITH_EIGEN=ON -DWITH_FFMPEG=ON -DWITH_GDAL=OFF -DWITH_GDCM=OFF -DWITH_GIGEAPI=OFF -DWITH_GPHOTO2=ON -DWITH_GSTREAMER=ON -DWITH_GSTREAMER_0_10=OFF -DWITH_GTK=ON -DWITH_GTK_2_X=OFF -DWITH_HALIDE=OFF -DWITH_HPX=OFF -DWITH_INF_ENGINE=ON -DWITH_ITT=OFF -DWITH_JASPER=ON -DWITH_JPEG=ON -DWITH_LAPACK=ON -DWITH_LIBREALSENSE=OFF -DWITH_MATLAB=OFF -DWITH_MFX=OFF -DWITH_NVCUVID=ON -DWITH_OPENCL=ON -DWITH_OPENCLAMDBLAS=ON -DWITH_OPENCLAMDFFT=ON -DWITH_OPENCL_SVM=OFF -DWITH_OPENEXR=ON -DWITH_OPENGL=ON -DWITH_OPENMP=OFF -DWITH_OPENNI=OFF -DWITH_OPENNI2=OFF -DWITH_OPENVX=OFF -DWITH_PNG=ON -DWITH_PROTOBUF=ON -DWITH_PTHREADS_PF=ON -DWITH_PVAPI=OFF -DWITH_QT=OFF -DWITH_TBB=OFF -DWITH_TIFF=ON -DWITH_V4L=ON -DWITH_VA=OFF -DWITH_VA_INTEL=OFF -DWITH_VTK=ON -DWITH_WEBP=ON -DWITH_XIMEA=OFF -DWITH_XINE=OFF -DBUILD_opencv_hdf=OFF ../

#Jetson Xavier
cmake -DBUILD_CUDA_STUBS=OFF -DBUILD_DOCS=OFF -DBUILD_EXAMPLES=OFF -DBUILD_JASPER=OFF -DBUILD_JAVA=OFF -DBUILD_JPEG=OFF -DBUILD_OPENEXR=OFF -DBUILD_PACKAGE=ON -DBUILD_PERF_TESTS=OFF -DBUILD_PNG=OFF -DBUILD_PROTOBUF=ON -DBUILD_SHARED_LIBS=ON -DBUILD_TBB=OFF -DBUILD_TESTS=OFF -DBUILD_TIFF=OFF -DBUILD_WITH_DEBUG_INFO=OFF -DBUILD_WITH_DYNAMIC_IPP=OFF -DBUILD_ZLIB=OFF -DBUILD_opencv_apps=ON -DBUILD_opencv_aruco=ON -DBUILD_opencv_bgsegm=ON -DBUILD_opencv_bioinspired=ON -DBUILD_opencv_calib3d=ON -DBUILD_opencv_ccalib=ON -DBUILD_opencv_core=ON -DBUILD_opencv_cudaarithm=ON -DBUILD_opencv_cudabgsegm=ON -DBUILD_opencv_cudacodec=ON -DBUILD_opencv_cudafeatures2d=ON -DBUILD_opencv_cudafilters=ON -DBUILD_opencv_cudaimgproc=ON -DBUILD_opencv_cudalegacy=OFF -DBUILD_opencv_cudaobjdetect=ON -DBUILD_opencv_cudaoptflow=ON -DBUILD_opencv_cudastereo=ON -DBUILD_opencv_cudawarping=ON -DBUILD_opencv_cudev=ON -DBUILD_opencv_datasets=ON -DBUILD_opencv_dnn=ON -DBUILD_opencv_dnn_objdetect=ON -DBUILD_opencv_dpm=ON -DBUILD_opencv_face=ON -DBUILD_opencv_features2d=ON -DBUILD_opencv_flann=ON -DBUILD_opencv_freetype=ON -DBUILD_opencv_fuzzy=OFF -DBUILD_opencv_highgui=ON -DBUILD_opencv_img_hash=ON -DBUILD_opencv_imgcodecs=ON -DBUILD_opencv_imgproc=ON -DBUILD_opencv_java_bindings_generator=OFF -DBUILD_opencv_js=OFF -DBUILD_opencv_js_bindings_generator=OFF -DBUILD_opencv_line_descriptor=ON -DBUILD_opencv_ml=ON -DBUILD_opencv_objdetect=ON -DBUILD_opencv_optflow=ON -DBUILD_opencv_phase_unwrapping=ON -DBUILD_opencv_photo=ON -DBUILD_opencv_plot=ON -DBUILD_opencv_python_bindings_generator=ON -DBUILD_opencv_python_tests=OFF -DBUILD_opencv_reg=ON -DBUILD_opencv_rgbd=ON -DBUILD_opencv_saliency=OFF -DBUILD_opencv_sfm=ON -DBUILD_opencv_shape=ON -DBUILD_opencv_stereo=ON -DBUILD_opencv_stitching=ON -DBUILD_opencv_structured_light=ON -DBUILD_opencv_superres=ON -DBUILD_opencv_surface_matching=ON -DBUILD_opencv_text=ON -DBUILD_opencv_tracking=ON -DBUILD_opencv_ts=ON -DBUILD_opencv_video=ON -DBUILD_opencv_videoio=ON -DBUILD_opencv_videostab=ON -DBUILD_opencv_world=OFF -DBUILD_opencv_xfeatures2d=OFF -DBUILD_opencv_ximgproc=ON -DBUILD_opencv_xobjdetect=ON -DBUILD_opencv_xphoto=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_CONFIGURATION_TYPES=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DCUDA_ARCH_BIN=7.2 -DCUDA_ARCH_PTX="" -DCUDA_FAST_MATH=ON -DENABLE_BUILD_HARDENING=OFF -DENABLE_FAST_MATH=ON -DENABLE_PRECOMPILED_HEADERS=OFF -DOPENCV_ENABLE_NONFREE=OFF -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules -DWITH_1394=OFF -DWITH_ARAVIS=OFF -DWITH_CAROTENE=ON -DWITH_CLP=OFF -DWITH_CUBLAS=ON -DWITH_CUDA=ON -DWITH_CUFFT=ON -DWITH_EIGEN=ON -DWITH_FFMPEG=ON -DWITH_GDAL=OFF -DWITH_GDCM=OFF -DWITH_GIGEAPI=OFF -DWITH_GPHOTO2=ON -DWITH_GSTREAMER=ON -DWITH_GSTREAMER_0_10=OFF -DWITH_GTK=ON -DWITH_GTK_2_X=OFF -DWITH_HALIDE=OFF -DWITH_HPX=OFF -DWITH_INF_ENGINE=ON -DWITH_ITT=OFF -DWITH_JASPER=ON -DWITH_JPEG=ON -DWITH_LAPACK=ON -DWITH_LIBREALSENSE=OFF -DWITH_MATLAB=OFF -DWITH_MFX=OFF -DWITH_NVCUVID=ON -DWITH_OPENCL=ON -DWITH_OPENCLAMDBLAS=ON -DWITH_OPENCLAMDFFT=ON -DWITH_OPENCL_SVM=OFF -DWITH_OPENEXR=ON -DWITH_OPENGL=ON -DWITH_OPENMP=OFF -DWITH_OPENNI=OFF -DWITH_OPENNI2=OFF -DWITH_OPENVX=OFF -DWITH_PNG=ON -DWITH_PROTOBUF=ON -DWITH_PTHREADS_PF=ON -DWITH_PVAPI=OFF -DWITH_QT=OFF -DWITH_TBB=OFF -DWITH_TIFF=ON -DWITH_V4L=ON -DWITH_VA=OFF -DWITH_VA_INTEL=OFF -DWITH_VTK=ON -DWITH_WEBP=ON -DWITH_XIMEA=OFF -DWITH_XINE=OFF -DBUILD_opencv_hdf=OFF ../

make all -j$(nproc)
sudo make install

#----------------------------------------------------
# (Optional) Jetson inference
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
# GCC
sudo apt-get -y install g++-9 gcc-9
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-9 9 --slave /usr/bin/g++ g++ /usr/bin/g++-9
sudo update-alternatives --config gcc

git clone --branch v0.13.0 --depth 1 --recursive https://github.com/intel-isl/Open3D
cd Open3D
git submodule update --init --recursive
mkdir build
cd build

#PC
cmake -DCMAKE_BUILD_TYPE=Release -DGLIBCXX_USE_CXX11_ABI=ON -DBUILD_CUDA_MODULE=ON -DBUILD_EXAMPLES=OFF -DBUILD_FILAMENT_FROM_SOURCE=ON -DBUILD_GUI=ON -DBUILD_PYTHON_MODULE=OFF -DBUILD_SHARED_LIBS=ON -DBUILD_WEBRTC=OFF -DDEVELOPER_BUILD=OFF -DWITH_SIMD=ON ../

# Jetson
sudo apt-get install -y apt-utils build-essential git cmake
sudo apt-get install -y python3 python3-dev python3-pip
sudo apt-get install -y xorg-dev libglu1-mesa-dev
sudo apt-get install -y libblas-dev liblapack-dev liblapacke-dev
sudo apt-get install -y libsdl2-dev libc++-7-dev libc++abi-7-dev libxi-dev
sudo apt-get install -y clang-7

# change default python to python3
python --version
sudo su
update-alternatives --install /usr/bin/python python /usr/bin/python3 1
exit
python --version

cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DGLIBCXX_USE_CXX11_ABI=ON \
    -DBUILD_CUDA_MODULE=ON \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_FILAMENT_FROM_SOURCE=ON \
    -DBUILD_GUI=ON \
    -DBUILD_PYTHON_MODULE=OFF \
    -DBUILD_SHARED_LIBS=ON \
    -DBUILD_TENSORFLOW_OPS=OFF \
    -DBUILD_UNIT_TESTS=OFF \
    -DDEVELOPER_BUILD=OFF \
    -DUSE_BLAS=ON \
    -DWITH_FAISS=OFF \
    -DWITH_IPPICV=OFF \
    -DWITH_SIMD=OFF \
    ..

make -j$(nproc)
sudo make install

# If met blas error
Open3D/cpp/open3d/core/linalg/BlasWrapper.h
#include "/usr/include/aarch64-linux-gnu/cblas-netlib.h" <-- add this
#include "open3d/core/linalg/LinalgHeadersCPU.h"

#----------------------------------------------------
# (Optional, use Open3D included version if possible) Filament
git clone --branch v1.9.9 --depth 1 https://github.com/google/filament.git
cd filament
mkdir out
mkdir out/release
cd out/release
CC=/usr/bin/clang CXX=/usr/bin/clang++ CXXFLAGS=-stdlib=libc++ cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../release/filament ../..
ninja -j6
ninja install
# need clang >= 7 for jetson!

# If error, try this
mkdir filament-binaries/lib/x86_64
cp filament-binaries/lib/*.a filament-binaries/lib/x86_64
make -j$(nproc)

#----------------------------------------------------
# (Optional) gwsocket
git clone --depth 1 https://github.com/allinurl/gwsocket.git
cd gwsocket
autoreconf -fiv
./configure
make -j$(nproc)

#----------------------------------------------------
# (Optional) Livox
git clone --depth 1 https://github.com/yankailab/Livox-SDK.git
cd Livox-SDK/build
cmake -DCMAKE_BUILD_TYPE=Release ../
make -j$(nproc)
sudo make install
# In direct connection mode configure the host ip to be 
# IP: 192.168.1.50
# Netmask: 255.255.255.0
# Gateway: 192.168.1.1

#----------------------------------------------------
# (Optional) Pangolin & Orb_Slam3
git clone --depth 1 https://github.com/stevenlovegrove/Pangolin.git
cd Pangolin
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
make all -j$(nproc)
sudo make install

git clone --depth 1 https://github.com/yankailab/ORB_SLAM3.git
cd ORB_SLAM3
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

#----------------------------------------------------
# (Optional) xArm
git clone --depth 1 https://github.com/xArm-Developer/xArm-CPLUS-SDK.git
cd xArm-CPLUS-SDK/
make all -j$(nproc)
sudo make install

#----------------------------------------------------
# OpenKAI
git clone --depth 1 https://github.com/yankailab/OpenKAI.git
cd OpenKAI
mkdir build
cd build
ccmake ../
make all -j$(nproc)

# Copy startup sh into home
sudo chmod a+x $HOME/ok.sh

# For startup on X, insert sleep before running OK to wait for X to be ready
sleep 5
./OK [kiss]

#----------------------------------------------------
# Misc.

# Upload to FTP
curl -T FourierToy.swf ftp://193.112.75.123/pub/ --user anonymous

# Test gstreamer
sudo gst-launch-1.0 v4l2src device=/dev/video0 ! video/x-raw,width=640,height=480,framerate=20/1 ! x264enc ! matroskamux ! filesink location=/home/pi/ssd/test.mkv
sudo gst-launch-1.0 v4l2src device=/dev/video0 ! video/x-raw,width=1280,height=720,framerate=30/1 ! videoconvert ! fbdevsink

# Screen and touch screen input rotate
xrandr -o left
xinput set-prop 'GXTP7386:00 27C6:0113' 'Coordinate Transformation Matrix' 0 -1 1 1 0 0 0 0 1

# Block unity touch gestures
sudo apt install dconf-editor
dconf-editor
com > canonical > unity > gestures

# Block on screen keyboard
sudo apt install gnome-tweaks
sudo apt install gnome-shell-extensions
https://extensions.gnome.org/extension/3222/block-caribou-36/

#----------------------------------------------------
# Make Jetson boot SD image

sudo fdisk -l
sudo umount /dev/sda
sudo dd if=/dev/sda of=~/sd.img bs=6M

sudo fdisk -l
sudo umount /dev/sdb
sudo dd if=~/sd.img of=/dev/sdb bs=6M


# Outdated
#----------------------------------------------------
# (Optional) Hypersen HPS3D
git clone --depth 1 https://github.com/hypersen/HPS3D_SDK.git
sudo mkdir /usr/local/include/HPS3D/
sudo cp HPS3D_SDK/English/api.h /usr/local/include/HPS3D/
sudo cp HPS3D_SDK/English/libhps3d64.so /usr/local/lib/
#sudo cp HPS3D_SDK/English/libhps3d_aarch64.so /usr/local/lib/
#sudo cp HPS3D_SDK/English/libhps3d_arm.so /usr/local/lib/

#----------------------------------------------------
# (Optional) MYNT EYE
git clone https://github.com/slightech/MYNT-EYE-D-SDK.git
cd MYNT-EYE-D-SDK
make init
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
make all -j12
sudo make install

#----------------------------------------------------
# (Optional) VTK
git clone --branch v8.2.0 --depth 1 https://gitlab.kitware.com/vtk/vtk.git
cd vtk
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF ../
make all -j12
sudo make install

#----------------------------------------------------
# (Optional) Tesseract
sudo apt-get -y install libleptonica-dev libcurl4-openssl-dev liblog4cplus-dev libpng-dev libjpeg8-dev libtiff5-dev zlib1g-dev
git clone https://github.com/tesseract-ocr/tesseract.git
cd tesseract
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
make all -j8
sudo make install

#----------------------------------------------------
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
