
# Update your system
```bash
sudo apt-get update
sudo apt-get upgrade
# (Raspberry pi)
sudo rasp-config
```


# Install prerequisites
```bash
sudo apt-get -y install build-essential cmake cmake-curses-gui git uuid-dev ncurses-dev libcurl4 curl libssl-dev libuvc-dev libusb-1.0-0-dev
```

## (Optional) Boost
```bash
sudo apt install --no-install-recommends libboost-dev libboost-system-dev libboost-thread-dev libboost-filesystem-dev
```

## (Optional) Video stream in/out functions
```bash
sudo apt-get -y install --no-install-recommends libunwind-dev gstreamer1.0-0 gstreamer1.0-plugins-base libgstreamer1.0-0 libgstreamer-plugins-base1.0-dev gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-libav gstreamer1.0-tools gstreamer1.0-alsa libv4l-dev v4l-utils libjpeg-dev libpng-dev libtiff-dev libavcodec-dev libavformat-dev libxvidcore-dev x264
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


# CMake
## Install from apt-get
```bash
sudo apt-get install cmake
```
## (Optional) Build from source for the latest
```bash
wget https://github.com/Kitware/CMake/releases/download/v3.31.12/cmake-3.31.12.tar.gz
tar xvf cmake-3.31.12.tar.gz
cd cmake-3.31.12

./bootstrap
make -j$(nproc)
sudo make install
bash
```


# Eigen 5
OpenKAI requires Eigen 5.x and builds with C++17. If Eigen 5 is already installed
with its CMake package, skip the installation commands.

```bash
wget https://gitlab.com/libeigen/eigen/-/archive/5.0.1/eigen-5.0.1.tar.gz
tar xf eigen-5.0.1.tar.gz
cd eigen-5.0.1

cmake -S . -B build \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DEIGEN_BUILD_BLAS=OFF \
    -DEIGEN_BUILD_LAPACK=OFF \
    -DEIGEN_BUILD_TESTING=OFF \
    -DBUILD_TESTING=OFF

sudo cmake --install build
```


# (Optional) RealSense
```bash
sudo apt-get install libusb-1.0-0-dev libusb-dev libudev-dev

git clone --depth 1 https://github.com/IntelRealSense/librealsense.git
# git clone --depth 1 --branch v2.53.1 https://github.com/IntelRealSense/librealsense.git # for T265, L535
cd librealsense
sudo ./scripts/setup_udev_rules.sh
mkdir build
cd build

# for Raspberry pi
cmake -DFORCE_LIBUVC=true -DFORCE_RSUSB_BACKEND=ON -DBUILD_WITH_TM2=true -DBUILD_WITH_CUDA=OFF -DCMAKE_BUILD_TYPE=Release
-DIMPORT_DEPTH_CAM_FW=ON ../

# for Jetson
cmake -DFORCE_LIBUVC=true -DFORCE_RSUSB_BACKEND=ON -DBUILD_WITH_TM2=true -DBUILD_WITH_CUDA=ON -DCMAKE_BUILD_TYPE=Release -DIMPORT_DEPTH_CAM_FW=ON ../

make -j$(nproc)
sudo make install

# USB reset if needed on auto boot
sudo uhubctl -a cycle -l 1 -p 1-4
sudo uhubctl -a cycle -l 2 -p 1-4

```


# (Optional) gphoto2
```bash
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
```


# (Optional) YOLO
See [YOLO26detectONNX.md](YOLO26detectONNX.md) for ONNX Runtime setup and `_YOLO26detectONNX` build/config notes.


# (Optional) OpenCV
See [opencv.md](opencv.md) for OpenCV build/config notes.


# (Optional) OrbbecSDK_v2
```bash
git clone --depth 1 https://github.com/orbbec/OrbbecSDK_v2.git
cd OrbbecSDK_v2
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DOB_BUILD_DOCS=OFF -DOB_BUILD_TOOLS=ON -DOB_INSTALL_EXAMPLES_SOURCE=OFF  ../
make -j$(nproc)
sudo make install

cd OrbbecSDK_v2/scripts/env_setup
sudo chmod +x ./install_udev_rules.sh
sudo ./install_udev_rules.sh
sudo udevadm control --reload-rules
sudo udevadm trigger
```


# (Optional) wsServer
```bash
git clone --depth 1 https://github.com/Theldus/wsServer
cd wsServer/
make
sudo make install
```


# (Optional) Livox
In direct connection mode configure the host ip
IP: 192.168.1.50
Netmask: 255.255.255.0
Gateway: 192.168.1.1


# (Optional) ScepterSDK
```bash
git clone --depth 1 https://github.com/ScepterSW/ScepterSDK.git
set +H
sudo sh -c "echo '[Path]/ScepterSDK/BaseSDK/Ubuntu/Lib' >> /etc/ld.so.conf"
set -H
sudo ldconfig
```
In direct connection mode configure the host ip
IP: 192.168.1.100
Netmask: 255.255.255.0
Gateway: 192.168.100.1


# (Optional) M4RI
```bash
sudo apt-get install autoconf automake libtool
git clone --depth 1 https://github.com/malb/m4ri.git
cd m4ri/
autoreconf --install
./configure
make
make check
sudo make install
```


# (Optional) OpenKAI extra modules
```bash
git clone --depth 1 https://github.com/yankailab/OpenKAI_exm.git
```


# OpenKAI
```bash
sudo apt-get -y install libunwind-dev libgoogle-glog-dev libevent-dev gsl-bin libgsl0-dev
git clone --depth 1 https://github.com/yankailab/OpenKAI.git
cd OpenKAI
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_AS_LIB=ON -DWITH_DEFAULT_MODULES=ON ../
make all -j$(nproc)
sudo make install
```
