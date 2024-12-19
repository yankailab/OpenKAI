
# Update your system
```bash
sudo apt-get update
sudo apt-get upgrade
# (Raspberry pi)
sudo rasp-config
```

# Install prerequisites
```bash
sudo apt-get -y install build-essential cmake cmake-curses-gui git uuid-dev ncurses-dev libcurl4 curl libssl-dev
```
## (Optional) Video stream in/out functions
```bash
sudo apt-get -y install libunwind-dev gstreamer1.0-0 gstreamer1.0-plugins-base libgstreamer1.0-0 libgstreamer-plugins-base1.0-dev gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-alsa gstreamer1.0-gl libv4l-dev v4l-utils libjpeg-dev libpng-dev libtiff-dev libavcodec-dev libavformat-dev libxvidcore-dev x264
```

## (Optional) Desktop renderings
```bash
sudo apt-get -y install libgtk-3-dev gstreamer1.0-x gstreamer1.0-gtk3
sudo apt-get -y install libglu1-mesa-dev libglu1-mesa libgl1-mesa-dev libglfw3 libglfw3-dev libglew-dev mesa-common-dev freeglut3-dev xorg-dev libxt-dev libxi-dev libx11-dev
```

## (Optional) Jpeg exif editing
```bash
sudo apt-get -y install libimage-exiftool-perl
```

# (Optional) CUDA
```bash
sudo apt install nvidia-cuda-toolkit
```
## (Optional) Select CUDA compatible GCC
```bash
sudo apt-get -y install g++-12 gcc-12
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 12 --slave /usr/bin/g++ g++ /usr/bin/g++-12
sudo update-alternatives --config gcc
```

# CMake
## Install from apt-get
```bash
sudo apt-get install cmake
```
## (Optional) Build from source for the latest
```bash
wget https://github.com/Kitware/CMake/releases/download/v3.31.2/cmake-3.31.2.tar.gz
tar xvf cmake-3.31.2.tar.gz
cd cmake-3.31.2
./bootstrap
make -j$(nproc)
sudo make install
bash
```

# Eigen
```bash
wget https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz
tar xvf eigen-3.4.0.tar.gz
cd eigen-3.4.0
mkdir build && cd build
cmake ../
sudo make install
```


# (Optional) RealSense
```bash
libusb-1.0-0-dev libusb-dev libudev-dev
```


# (Optional) OpenCV
General CMake configs for x86-64 machines
```bash
sudo apt-get -y install libprotobuf-dev protobuf-compiler
git clone --branch 4.10.0 --depth 1 https://github.com/opencv/opencv.git
git clone --branch 4.10.0 --depth 1 https://github.com/opencv/opencv_contrib.git
cd opencv
mkdir build && cd build
cmake -DBUILD_CUDA_STUBS=OFF \
      -DBUILD_DOCS=OFF \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_IPP_IW=OFF \
      -DBUILD_ITT=OFF \
      -DBUILD_JASPER=OFF \
      -DBUILD_JAVA=OFF \
      -DBUILD_JPEG=OFF \
      -DBUILD_OPENEXR=OFF \
      -DBUILD_PACKAGE=ON \
      -DBUILD_PERF_TESTS=OFF \
      -DBUILD_PNG=OFF \
      -DBUILD_PROTOBUF=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TBB=OFF \
      -DBUILD_TESTS=OFF \
      -DBUILD_TIFF=OFF \
      -DBUILD_WEBP=OFF \
      -DBUILD_WITH_DEBUG_INFO=OFF \
      -DBUILD_WITH_DYNAMIC_IPP=OFF \
      -DBUILD_ZLIB=OFF \
      -DBUILD_opencv_alphamat=ON \
      -DBUILD_opencv_apps=ON \
      -DBUILD_opencv_aruco=ON \
      -DBUILD_opencv_barcode=ON \
      -DBUILD_opencv_bgsegm=ON \
      -DBUILD_opencv_bioinspired=ON \
      -DBUILD_opencv_calib3d=ON \
      -DBUILD_opencv_ccalib=ON \
      -DBUILD_opencv_core=ON \
      -DBUILD_opencv_datasets=ON \
      -DBUILD_opencv_dnn=ON \
      -DBUILD_opencv_dnn_objdetect=ON \
      -DBUILD_opencv_dnn_superres=ON \
      -DBUILD_opencv_dpm=ON \
      -DBUILD_opencv_face=ON \
      -DBUILD_opencv_features2d=ON \
      -DBUILD_opencv_flann=ON \
      -DBUILD_opencv_fuzzy=ON \
      -DBUILD_opencv_gapi=ON \
      -DBUILD_opencv_hdf=OFF \
      -DBUILD_opencv_hfs=ON \
      -DBUILD_opencv_highgui=ON \
      -DBUILD_opencv_img_hash=ON \
      -DBUILD_opencv_imgcodecs=ON \
      -DBUILD_opencv_imgproc=ON \
      -DBUILD_opencv_java_bindings_generator=OFF \
      -DBUILD_opencv_js=OFF \
      -DBUILD_opencv_js_bindings_generator=OFF \
      -DBUILD_opencv_line_descriptor=ON \
      -DBUILD_opencv_ml=ON \
      -DBUILD_opencv_objc_bindings_generator=OFF \
      -DBUILD_opencv_objdetect=ON \
      -DBUILD_opencv_optflow=ON \
      -DBUILD_opencv_phase_unwrapping=ON \
      -DBUILD_opencv_photo=ON \
      -DBUILD_opencv_plot=ON \
      -DBUILD_opencv_python3=OFF \
      -DBUILD_opencv_python_bindings_generator=OFF \
      -DBUILD_opencv_python_tests=OFF \
      -DBUILD_opencv_reg=ON \
      -DBUILD_opencv_rgbd=ON \
      -DBUILD_opencv_saliency=ON \
      -DBUILD_opencv_sfm=ON \
      -DBUILD_opencv_shape=ON \
      -DBUILD_opencv_stereo=ON \
      -DBUILD_opencv_stitching=ON \
      -DBUILD_opencv_structured_light=ON \
      -DBUILD_opencv_superres=ON \
      -DBUILD_opencv_surface_matching=ON \
      -DBUILD_opencv_text=ON \
      -DBUILD_opencv_tracking=ON \
      -DBUILD_opencv_ts=ON \
      -DBUILD_opencv_video=ON \
      -DBUILD_opencv_videoio=ON \
      -DBUILD_opencv_videostab=ON \
      -DBUILD_opencv_world=OFF \
      -DBUILD_opencv_wechat_qrcode=OFF \
      -DBUILD_opencv_xfeatures2d=ON \
      -DBUILD_opencv_ximgproc=ON \
      -DBUILD_opencv_xobjdetect=ON \
      -DBUILD_opencv_xphoto=ON \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CONFIGURATION_TYPES=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DENABLE_BUILD_HARDENING=OFF \
      -DENABLE_FAST_MATH=ON \
      -DENABLE_PRECOMPILED_HEADERS=OFF \
      -DOPENCV_ENABLE_NONFREE=OFF \
      -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules \
      -DOPENCV_DNN_OPENVINO=OFF \
      -DOPENCV_DNN_TFLITE=ON \
      -DOPENCV_GAPI_GSTREAMER=ON \
      -DOPJ_USE_THREAD=ON \
      -DPROTOBUF_UPDATE_FILES=ON \
      -DWITH_1394=OFF \
      -DWITH_ARAVIS=OFF \
      -DWITH_CLP=OFF \
      -DWITH_EIGEN=ON \
      -DWITH_FFMPEG=ON \
      -DWITH_FREETYPE=OFF \
      -DWITH_GDAL=OFF \
      -DWITH_GDCM=OFF \
      -DWITH_GPHOTO2=ON \
      -DWITH_GSTREAMER=ON \
      -DWITH_GTK=ON \
      -DWITH_GTK_2_X=OFF \
      -DWITH_HALIDE=OFF \
      -DWITH_HPX=OFF \
      -DWITH_INF_ENGINE=ON \
      -DWITH_IPP=ON \
      -DWITH_ITT=ON \
      -DWITH_JASPER=ON \
      -DWITH_JPEG=ON \
      -DWITH_LAPACK=ON \
      -DWITH_LIBREALSENSE=OFF \
      -DWITH_MATLAB=OFF \
      -DWITH_MFX=OFF \
      -DWITH_OPENCL=ON \
      -DWITH_OPENCLAMDBLAS=ON \
      -DWITH_OPENCLAMDFFT=ON \
      -DWITH_OPENCL_SVM=OFF \
      -DWITH_OPENEXR=ON \
      -DWITH_OPENGL=ON \
      -DWITH_OPENMP=OFF \
      -DWITH_OPENNI=OFF \
      -DWITH_OPENNI2=OFF \
      -DWITH_OPENVINO=OFF \
      -DWITH_OPENVX=OFF \
      -DWITH_PNG=ON \
      -DWITH_PROTOBUF=ON \
      -DWITH_PTHREADS_PF=ON \
      -DWITH_PVAPI=OFF \
      -DWITH_QT=OFF \
      -DWITH_TBB=ON \
      -DWITH_TIFF=ON \
      -DWITH_V4L=ON \
      -DWITH_VA=OFF \
      -DWITH_VA_INTEL=OFF \
      -DWITH_VTK=ON \
      -DWITH_WEBP=ON \
      -DWITH_XIMEA=OFF \
      -DWITH_XINE=OFF \
      ../

# (Optional) with CUDA
-DWITH_CUDA=ON \
-DCUDA_ARCH_BIN=8.6 \
-DCUDA_ARCH_PTX=8.6 \
-DCUDA_FAST_MATH=ON \

make all -j$(nproc)
sudo make install
sudo ldconfig
```

For Jetson and Raspberry pi
```bash
sudo apt-get -y install libprotobuf-dev protobuf-compiler
git clone --branch 4.10.0 --depth 1 https://github.com/opencv/opencv.git
git clone --branch 4.10.0 --depth 1 https://github.com/opencv/opencv_contrib.git
cd opencv
mkdir build && cd build
cmake \
      -DBUILD_CUDA_STUBS=OFF \
      -DBUILD_DOCS=OFF \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_IPP_IW=OFF \
      -DBUILD_ITT=OFF \
      -DBUILD_JASPER=OFF \
      -DBUILD_JAVA=OFF \
      -DBUILD_JPEG=OFF \
      -DBUILD_OPENEXR=OFF \
      -DBUILD_PACKAGE=ON \
      -DBUILD_PERF_TESTS=OFF \
      -DBUILD_PNG=OFF \
      -DBUILD_PROTOBUF=ON \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TBB=OFF \
      -DBUILD_TESTS=OFF \
      -DBUILD_TIFF=OFF \
      -DBUILD_WEBP=OFF \
      -DBUILD_WITH_DEBUG_INFO=OFF \
      -DBUILD_WITH_DYNAMIC_IPP=OFF \
      -DBUILD_ZLIB=OFF \
      -DBUILD_opencv_alphamat=ON \
      -DBUILD_opencv_apps=ON \
      -DBUILD_opencv_aruco=ON \
      -DBUILD_opencv_barcode=ON \
      -DBUILD_opencv_bgsegm=ON \
      -DBUILD_opencv_bioinspired=ON \
      -DBUILD_opencv_calib3d=ON \
      -DBUILD_opencv_ccalib=ON \
      -DBUILD_opencv_core=ON \
      -DBUILD_opencv_datasets=ON \
      -DBUILD_opencv_dnn=ON \
      -DBUILD_opencv_dnn_objdetect=ON \
      -DBUILD_opencv_dnn_superres=ON \
      -DBUILD_opencv_dpm=ON \
      -DBUILD_opencv_face=ON \
      -DBUILD_opencv_features2d=ON \
      -DBUILD_opencv_flann=ON \
      -DBUILD_opencv_fuzzy=ON \
      -DBUILD_opencv_gapi=ON \
      -DBUILD_opencv_hdf=OFF \
      -DBUILD_opencv_hfs=ON \
      -DBUILD_opencv_highgui=ON \
      -DBUILD_opencv_img_hash=ON \
      -DBUILD_opencv_imgcodecs=ON \
      -DBUILD_opencv_imgproc=ON \
      -DBUILD_opencv_java_bindings_generator=OFF \
      -DBUILD_opencv_js=OFF \
      -DBUILD_opencv_js_bindings_generator=OFF \
      -DBUILD_opencv_line_descriptor=ON \
      -DBUILD_opencv_ml=ON \
      -DBUILD_opencv_objc_bindings_generator=OFF \
      -DBUILD_opencv_objdetect=ON \
      -DBUILD_opencv_optflow=ON \
      -DBUILD_opencv_phase_unwrapping=ON \
      -DBUILD_opencv_photo=ON \
      -DBUILD_opencv_plot=ON \
      -DBUILD_opencv_python3=OFF \
      -DBUILD_opencv_python_bindings_generator=OFF \
      -DBUILD_opencv_python_tests=OFF \
      -DBUILD_opencv_reg=ON \
      -DBUILD_opencv_rgbd=ON \
      -DBUILD_opencv_saliency=ON \
      -DBUILD_opencv_sfm=ON \
      -DBUILD_opencv_shape=ON \
      -DBUILD_opencv_stereo=ON \
      -DBUILD_opencv_stitching=ON \
      -DBUILD_opencv_structured_light=ON \
      -DBUILD_opencv_superres=ON \
      -DBUILD_opencv_surface_matching=ON \
      -DBUILD_opencv_text=ON \
      -DBUILD_opencv_tracking=ON \
      -DBUILD_opencv_ts=ON \
      -DBUILD_opencv_video=ON \
      -DBUILD_opencv_videoio=ON \
      -DBUILD_opencv_videostab=ON \
      -DBUILD_opencv_world=OFF \
      -DBUILD_opencv_wechat_qrcode=OFF \
      -DBUILD_opencv_xfeatures2d=ON \
      -DBUILD_opencv_ximgproc=ON \
      -DBUILD_opencv_xobjdetect=ON \
      -DBUILD_opencv_xphoto=ON \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CONFIGURATION_TYPES=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DENABLE_BUILD_HARDENING=OFF \
      -DENABLE_FAST_MATH=ON \
      -DENABLE_PRECOMPILED_HEADERS=OFF \
      -DOPENCV_ENABLE_NONFREE=OFF \
      -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules \
      -DOPENCV_DNN_OPENVINO=OFF \
      -DOPENCV_DNN_TFLITE=ON \
      -DOPENCV_GAPI_GSTREAMER=ON \
      -DOPJ_USE_THREAD=ON \
      -DPROTOBUF_UPDATE_FILES=OFF \
      -DWITH_1394=OFF \
      -DWITH_ARAVIS=OFF \
      -DWITH_CLP=OFF \
      -DWITH_EIGEN=ON \
      -DWITH_FFMPEG=ON \
      -DWITH_FREETYPE=OFF \
      -DWITH_GDAL=OFF \
      -DWITH_GDCM=OFF \
      -DWITH_GPHOTO2=ON \
      -DWITH_GSTREAMER=ON \
      -DWITH_GTK=OFF \
      -DWITH_GTK_2_X=OFF \
      -DWITH_HALIDE=OFF \
      -DWITH_HPX=OFF \
      -DWITH_INF_ENGINE=ON \
      -DWITH_IPP=OFF \
      -DWITH_ITT=OFF \
      -DWITH_JASPER=ON \
      -DWITH_JPEG=ON \
      -DWITH_LAPACK=ON \
      -DWITH_LIBREALSENSE=OFF \
      -DWITH_MATLAB=OFF \
      -DWITH_MFX=OFF \
      -DWITH_OPENCL=ON \
      -DWITH_OPENCLAMDBLAS=ON \
      -DWITH_OPENCLAMDFFT=ON \
      -DWITH_OPENCL_SVM=OFF \
      -DWITH_OPENEXR=ON \
      -DWITH_OPENGL=ON \
      -DWITH_OPENMP=OFF \
      -DWITH_OPENNI=OFF \
      -DWITH_OPENNI2=OFF \
      -DWITH_OPENVINO=OFF \
      -DWITH_OPENVX=OFF \
      -DWITH_PNG=ON \
      -DWITH_PROTOBUF=ON \
      -DWITH_PTHREADS_PF=ON \
      -DWITH_PVAPI=OFF \
      -DWITH_QT=OFF \
      -DWITH_TBB=OFF \
      -DWITH_TIFF=ON \
      -DWITH_V4L=ON \
      -DWITH_VA=OFF \
      -DWITH_VA_INTEL=OFF \
      -DWITH_VTK=ON \
      -DWITH_WEBP=ON \
      -DWITH_XIMEA=OFF \
      -DWITH_XINE=OFF \
      ../

make all -j$(nproc)
sudo make install
sudo ldconfig
```

# (Optional) Chilitags
```bash
git clone --depth 1 https://github.com/chili-epfl/chilitags.git
cd chilitags
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
make -j$(nproc)
sudo make install
```

# (Optional) Open3D
```bash
sudo apt-get -y install libjsoncpp-dev libc++1 gfortran
sudo apt-get -y install python3 python3-pip
sudo apt-get -y install xorg-dev libxcb-shm0 libglu1-mesa-dev libosmesa6-dev

git clone --branch v0.18.0 --depth 1 --recursive https://github.com/intel-isl/Open3D
cd Open3D
git submodule update --init --recursive
```

## (Optional) Build Filament from source
```bash
sudo apt-get -y install clang libc++-dev libc++abi-dev libsdl2-dev ninja-build libxi-dev
    # ML
    libtbb-dev
    # Headless rendering
    libosmesa6-dev
    # RealSense
    libudev-dev
    autoconf
    libtool
# For ARM64
    gfortran
```

## Build and install
```bash
mkdir build && cd build

# (desktop)
cmake -DCMAKE_BUILD_TYPE=Release \
      -DGLIBCXX_USE_CXX11_ABI=ON \
      -DBUILD_CUDA_MODULE=OFF \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_FILAMENT_FROM_SOURCE=OFF \
      -DBUILD_GUI=ON \
      -DBUILD_PYTHON_MODULE=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TENSORFLOW_OPS=OFF \
      -DBUILD_WEBRTC=ON \
      -DBUILD_UNIT_TESTS=OFF \
      -DDEVELOPER_BUILD=OFF \
      -DWITH_SIMD=ON ../

# (Raspberry pi)
cmake -DCMAKE_BUILD_TYPE=Release \
      -DGLIBCXX_USE_CXX11_ABI=ON \
      -DBUILD_CUDA_MODULE=OFF \
      -DBUILD_EXAMPLES=OFF \
      -DBUILD_GUI=OFF \
      -DBUILD_PYTHON_MODULE=OFF \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TENSORFLOW_OPS=OFF \
      -DBUILD_WEBRTC=OFF \
      -DBUILD_UNIT_TESTS=OFF \
      -DDEVELOPER_BUILD=OFF \
      -DWITH_SIMD=OFF \
      -DWITH_IPPICV=OFF \
      ../

make -j$(nproc)
sudo make install
```

# (Optional) wsServer
```bash
git clone https://github.com/Theldus/wsServer
cd wsServer/
make
sudo make install
```

# (Optional) OpenKAI extra modules
```bash
git clone --depth 1 https://github.com/yankailab/OpenKAI_exm.git
```

# OpenKAI
```bash
sudo apt-get -y install libgoogle-glog-dev libevent-dev gsl-bin libgsl0-dev
git clone --depth 1 https://github.com/yankailab/OpenKAI.git
cd OpenKAI
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_AS_LIB=ON -DWITH_DEFAULT_MODULES=ON ../
make all -j$(nproc)
```
