# (Optional) Open3D
```bash
git clone --branch v0.19.0 --depth 1 --recursive https://github.com/intel-isl/Open3D
cd Open3D
git submodule update --init --recursive
mkdir build && cd build
```

## Desktop
Install gcc-11 on Ubuntu 24.04 if met compile error.
(Optional) Build Filament from source
```bash
sudo apt-get -y install clang libsdl2-dev libxi-dev
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

Build and install
```bash
sudo apt-get -y install libjsoncpp-dev libc++1 gfortran libfmt-dev
sudo apt-get -y install xorg-dev libglu1-mesa-dev libxcb-shm0
sudo apt-get -y install python3 python3-pip
sudo apt-get -y install libc++-dev libc++abi-dev ninja-build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DGLIBCXX_USE_CXX11_ABI=ON \
      -DUSE_SYSTEM_EIGEN3=ON \
      -DEigen3_DIR=/usr/local/share/eigen3/cmake \
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

make -j$(nproc)
sudo make install
```

## For Raspberry pi headless rendering,
Patch this file:
```bash
/home/lab/dev/Open3D/3rdparty/glew/src/glew.c
```

Immediately before:
```c
#include <GL/osmesa.h>
```

Make that block look like this:
```c
#ifndef GLAPI
#define GLAPI extern
#endif

#ifndef GLAPIENTRY
#define GLAPIENTRY
#endif

#ifndef APIENTRY
#define APIENTRY
#endif

#include <GL/osmesa.h>
```

```bash
sudo apt-get -y install libjsoncpp-dev libc++1 gfortran libfmt-dev
sudo apt-get -y install xorg-dev libglu1-mesa-dev
sudo apt-get -y install libc++-dev libc++abi-dev ninja-build

sudo apt-get -y install libglew-dev libglfw3-dev libosmesa6-dev libxkbcommon-dev

cmake -DCMAKE_BUILD_TYPE=Release \
      -DGLIBCXX_USE_CXX11_ABI=ON \
      -DUSE_SYSTEM_EIGEN3=ON \
      -DEigen3_DIR=/usr/local/share/eigen3/cmake \
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
      -DENABLE_HEADLESS_RENDERING=ON \
      -DUSE_SYSTEM_GLEW=OFF \
      -DUSE_SYSTEM_GLFW=OFF \
      -DWITH_IPP=OFF \
      ../

make -j$(nproc)
sudo make install
```
