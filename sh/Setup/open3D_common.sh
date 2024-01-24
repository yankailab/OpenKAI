#----------------------------------------------------
# (Optional) Open3D
sudo apt-get install libjsoncpp-dev
#git clone --branch v0.18.0 --depth 1 --recursive https://github.com/intel-isl/Open3D
git clone --branch GuiWinSetFullScr --depth 1 --recursive https://github.com/yankailab/Open3D
cd Open3D
git submodule update --init --recursive
# run this again if met error in re-compile
util/install_deps_ubuntu.sh
mkdir build
cd build

#PC
cmake -DCMAKE_BUILD_TYPE=Release -DGLIBCXX_USE_CXX11_ABI=ON -DBUILD_CUDA_MODULE=OFF -DBUILD_WEBRTC=ON -DBUILD_EXAMPLES=OFF -DBUILD_FILAMENT_FROM_SOURCE=OFF -DBUILD_GUI=ON -DBUILD_PYTHON_MODULE=OFF -DBUILD_SHARED_LIBS=ON -DBUILD_WEBRTC=OFF -DDEVELOPER_BUILD=OFF -DWITH_SIMD=ON ../
make -j$(nproc)
sudo make install



# TODO: Raspberry pi
util/install_deps_ubuntu.sh
sudo apt-get install -y xorg-dev libglu1-mesa-dev
sudo apt-get install libglew-dev

cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=ON \
    -DGLIBCXX_USE_CXX11_ABI=ON \
    -DBUILD_CUDA_MODULE=OFF \
    -DBUILD_EXAMPLES=OFF \
    -DBUILD_FILAMENT_FROM_SOURCE=ON \
    -DBUILD_GUI=OFF \
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

#TODO: Jetson
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
