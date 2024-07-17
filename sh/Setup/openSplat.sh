#----------------------------------------------------
# ffmpeg
sudo apt-get install ffmpeg
ffmpeg -i IMG_5087.MOV -vf "fps=2" %06d.jpg

# COLMAP
# https://colmap.github.io/install.html
# For CUDA verified with CUDA 12.3 with gcc 11 under Ubuntu 22.04

sudo apt-get install \
    ninja-build \
    libboost-program-options-dev \
    libboost-filesystem-dev \
    libboost-graph-dev \
    libboost-system-dev \
    libflann-dev \
    libfreeimage-dev \
    libmetis-dev \
    libgoogle-glog-dev \
    libgtest-dev \
    libsqlite3-dev \
    libglew-dev \
    qtbase5-dev \
    libqt5opengl5-dev \
    libcgal-dev \
    libceres-dev

git clone https://github.com/colmap/colmap.git
cd colmap
mkdir build
cd build
cmake .. -GNinja
ninja
sudo ninja install

./colmap/build/src/colmap/exe/colmap gui

# Libtorch (Pytorch C++)
https://pytorch.org/cppdocs/installing.html

# Download and extract to somewhere
# Using CUDA 12.3 with LibTorch (CUDA 12.1)
https://pytorch.org/get-started/locally/#linux-installation

# put the absolute path to CMAKE_PREFIX_PATH
#mkdir build
#cd build
#cmake -DCMAKE_PREFIX_PATH=/absolute/path/to/libtorch ..
#cmake --build . --config Release

# OpenSplat
# CPU
git clone https://github.com/pierotofy/OpenSplat
cd OpenSplat
mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH=/path/to/libtorch/ ..
make -j$(nproc)

# CUDA
# CUDA: Make sure you have the CUDA compiler (nvcc) in your PATH and that nvidia-smi is working. https://docs.nvidia.com/cuda/cuda-installation-guide-linux/index.html
# For libtorch visit https://pytorch.org/get-started/locally/ and select your OS, for package select "LibTorch". Make sure to match your version of CUDA if you want to leverage GPU support in libtorch.
git clone https://github.com/pierotofy/OpenSplat
cd OpenSplat
mkdir build && cd build
cmake -DCMAKE_PREFIX_PATH=/path/to/libtorch/ ..
make -j$(nproc)

# Run
cd build
./opensplat /path/to/colmap_project_dir -n 2000

# Viewer
https://playcanvas.com/viewer
https://playcanvas.com/supersplat/editor