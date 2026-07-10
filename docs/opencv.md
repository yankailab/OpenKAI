# (Optional) OpenCV
## General CMake configs for x86-64 machines
```bash
sudo apt-get -y install libprotobuf-dev protobuf-compiler libgoogle-glog-dev
git clone --branch 5.0.0 --depth 1 https://github.com/opencv/opencv.git
git clone --branch 5.0.0 --depth 1 https://github.com/opencv/opencv_contrib.git
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
      -DBUILD_PROTOBUF=ON \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TBB=OFF \
      -DBUILD_TESTS=OFF \
      -DBUILD_TIFF=OFF \
      -DBUILD_WEBP=OFF \
      -DBUILD_WITH_DEBUG_INFO=OFF \
      -DBUILD_WITH_DYNAMIC_IPP=OFF \
      -DBUILD_ZLIB=OFF \
      -DBUILD_opencv_java_bindings_generator=OFF \
      -DBUILD_opencv_js=OFF \
      -DBUILD_opencv_js_bindings_generator=OFF \
      -DBUILD_opencv_objc_bindings_generator=OFF \
      -DBUILD_opencv_python_bindings_generator=OFF \
      -DBUILD_opencv_python_tests=OFF \
      -DCMAKE_BUILD_TYPE=Release \
      -DENABLE_BUILD_HARDENING=OFF \
      -DENABLE_FAST_MATH=ON \
      -DENABLE_PRECOMPILED_HEADERS=OFF \
      -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules \
      -DOPENCV_GAPI_GSTREAMER=ON \
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
      -DWITH_GTK=ON \
      -DWITH_GTK_2_X=OFF \
      -DWITH_HPX=OFF \
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

## For Jetson and Raspberry pi
```bash
sudo apt-get -y install libprotobuf-dev protobuf-compiler libgoogle-glog-dev
git clone --branch 5.0.0 --depth 1 https://github.com/opencv/opencv.git
git clone --branch 5.0.0 --depth 1 https://github.com/opencv/opencv_contrib.git
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
      -DBUILD_PROTOBUF=ON \
      -DBUILD_SHARED_LIBS=ON \
      -DBUILD_TBB=OFF \
      -DBUILD_TESTS=OFF \
      -DBUILD_TIFF=OFF \
      -DBUILD_WEBP=OFF \
      -DBUILD_WITH_DEBUG_INFO=OFF \
      -DBUILD_WITH_DYNAMIC_IPP=OFF \
      -DBUILD_ZLIB=OFF \
      -DBUILD_opencv_java_bindings_generator=OFF \
      -DBUILD_opencv_js=OFF \
      -DBUILD_opencv_js_bindings_generator=OFF \
      -DBUILD_opencv_objc_bindings_generator=OFF \
      -DBUILD_opencv_python_bindings_generator=OFF \
      -DBUILD_opencv_python_tests=OFF \
      -DCMAKE_BUILD_TYPE=Release \
      -DENABLE_BUILD_HARDENING=OFF \
      -DENABLE_FAST_MATH=ON \
      -DENABLE_PRECOMPILED_HEADERS=OFF \
      -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules \
      -DOPENCV_GAPI_GSTREAMER=ON \
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
      -DWITH_GTK=ON \
      -DWITH_GTK_2_X=OFF \
      -DWITH_HPX=OFF \
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
      -DWITH_VTK=OFF \
      -DWITH_WEBP=ON \
      -DWITH_XIMEA=OFF \
      -DWITH_XINE=OFF \
      ../

make all -j$(nproc)
sudo make install
sudo ldconfig
```

