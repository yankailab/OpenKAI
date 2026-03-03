# Misc.

# Update kernel
```bash
uname -r
wget https://kernel.ubuntu.com/mainline/v6.6.7/amd64/linux-headers-6.6.7-060607-generic_6.6.7-060607.202312131837_amd64.deb
wget https://kernel.ubuntu.com/mainline/v6.6.7/amd64/linux-headers-6.6.7-060607_6.6.7-060607.202312131837_all.deb
wget https://kernel.ubuntu.com/mainline/v6.6.7/amd64/linux-image-unsigned-6.6.7-060607-generic_6.6.7-060607.202312131837_amd64.deb
wget https://kernel.ubuntu.com/mainline/v6.6.7/amd64/linux-modules-6.6.7-060607-generic_6.6.7-060607.202312131837_amd64.deb
sudo dpkg -i *.deb
sudo reboot now
```


# ssh
```bash
sudo apt-get install openssh-server -y
sudo systemctl start ssh
sudo systemctl enable ssh
sudo systemctl status ssh
```


# Openconnect
```bash
sudo apt-get install openconnect
#sudo apt install network-manager-openconnect-gnome
sudo openconnect -u username -b server.com
ip a
```


# System setup
```bash
sudo apt-get update
sudo apt-get upgrade
sudo apt-get --with-new-pkgs upgrade
sudo aptitude full-upgrade
sudo apt-get update
sudo apt-get upgrade
#sudo apt-get dist-upgrade
sudo apt autoremove
sudo apt clean
```


# Numerical
```bash
sudo apt-get -y install libqhull-dev qhull-bin gfortran libblas-dev liblapack-dev liblapacke-dev liblapack3 libatlas-base-dev libopenblas-base libopenblas-dev libflann-dev libproj-dev
```


# Graph plotting
```bash
sudo apt-get -y install libmgl-dev
```


# (Optional)
```bash
sudo apt-get -y install python3 python3-pip
pip install --upgrade google-api-python-client google-auth-httplib2 google-auth-oauthlib
snap install cloudcompare
```


# (Optional) Jetson inference
CuDNN, download the latest .deb from NVIDIA site
```bash
sudo dpkg -i cudnn-local-repo-ubuntu2204-8.8.0.121_1.0-1_amd64.deb
```

# TensorRT

# Jetson Nano/Xavier
```bash
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
```


# (Optional) xArm
```bash
git clone --depth 1 https://github.com/xArm-Developer/xArm-CPLUS-SDK.git
cd xArm-CPLUS-SDK/
make all -j$(nproc)
sudo make install
```


# (Optional) Pangolin & Orb_Slam3
```bash
git clone --branch v0.8 --depth 1 https://github.com/stevenlovegrove/Pangolin.git
cd Pangolin
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ../
make all -j$(nproc)
sudo make install

#git clone --depth 1 https://github.com/yankailab/ORB_SLAM3.git
git clone --branch v1.0-release --depth 1 https://github.com/UZ-SLAMLab/ORB_SLAM3.git
cd ORB_SLAM3
sed -i 's/++11/++14/g' CMakeLists.txt
chmod +x build.sh
./build.sh
# sudo make install
```


# (Optional) Dynamixel
```bash
git clone --depth 1 https://github.com/ROBOTIS-GIT/DynamixelSDK.git
cd DynamixelSDK/c++/build/linux64
make -j$(nproc)
sudo make install
# For Jetson and Raspberry Pi
# remove "-m64" in "FORMAT" line of the Makefile in linux64 directory
```
