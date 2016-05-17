# OpenKAI

//-------------------------------------------------------------
//GitHub Repositories
sudo apt-get install git
git clone https://github.com/Itseez/opencv.git
git clone https://github.com/Itseez/opencv_contrib.git
git clone https://github.com/yankailab/OpenKAI.git
//Optional for DNN
git clone https://github.com/BVLC/caffe.git

//-------------------------------------------------------------
//g++
sudo apt-get install build-essential

//-------------------------------------------------------------
//CUDA7.5
sudo gedit ~/.bashrc
export PATH=/usr/local/cuda-7.5/bin:$PATH
export LD_LIBRARY_PATH=/usr/local/cuda-7.5/lib64:$LD_LIBRARY_PATH
//Test
nvcc --version
nvidia-smi

//-------------------------------------------------------------
//CuDNN
cd cuda
sudo cp lib64/lib* /usr/local/cuda/lib64/
sudo cp include/cudnn.h /usr/local/cuda/include/

//-------------------------------------------------------------
//OpenCV
sudo apt-get -y install libgtk2.0-dev pkg-config python-dev python-numpy libdc1394-22 libdc1394-22-dev libjpeg-dev libpng12-dev libtiff4-dev libjasper-dev libavcodec-dev libavformat-dev libswscale-dev libxine-dev libgstreamer0.10-dev libgstreamer-plugins-base0.10-dev libv4l-dev libtbb-dev libqt4-dev libfaac-dev libmp3lame-dev libopencore-amrnb-dev libopencore-amrwb-dev libtheora-dev libvorbis-dev libxvidcore-dev x264 v4l-utils unzip python-protobuf python-scipy python-pip

//OpenCV cmake
cd build
sudo make all -j8
sudo make install
//export PATH=/usr/local/lib:$PATH

//Caffe
sudo apt-get install libatlas-base-dev libprotobuf-dev libleveldb-dev libsnappy-dev libboost-all-dev libhdf5-serial-dev libgflags-dev libgoogle-glog-dev liblmdb-dev protobuf-compiler

//Compile Caffe
make all -j8
make test -j8

//Add to ldconfig
sudo gedit /etc/ld.so.conf
/usr/local/cuda/lib64
/home/kai/dev/caffe-segnet/build
sudo ldconfig

make runtest
make pycaffe

//Python interface
sudo pip install matplotlib
sudo pip install scikit-image

//Build OpenKAI
./build.sh
