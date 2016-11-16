cmake_minimum_required(VERSION 2.8)
set(CMAKE_BUILD_TYPE Release)
project(OpenKAI)
add_definitions("-O3 -w -Wall -c -fmessage-length=0 -MMD -MP -std=c++11")

# Setting specific lib here
link_directories(/home/ubuntu/src/jetson-inference/build/aarch64/lib /usr/local/lib /usr/local/opencv3/lib /home/ubuntu/src/caffe/build/lib /home/ubuntu/src/caffe/build)

# setup CUDA
find_package(CUDA)

set(
	CUDA_NVCC_FLAGS
	${CUDA_NVCC_FLAGS}; 
    -O3 -gencode arch=compute_53,code=sm_53
)

#file(GLOB OpenKAIcpp )
file(GLOB OpenKAIcpp src/*.cpp src/*/*.cpp src/*/*/*.cpp src/*/*/*/*.cpp)

cuda_add_executable(OpenKAI ${OpenKAIcpp})

# C/C++ settings
include_directories(/home/ubuntu/src/jetson-inference/cuda /home/ubuntu/src/jetson-inference/build/aarch64/include /home/ubuntu/src/caffe/include /home/ubuntu/src/caffe/build/src /usr/local/cuda/include /usr/local/include /usr/local/opencv3/include)

target_link_libraries(OpenKAI pthread boost_system glog caffe opencv_core opencv_imgcodecs opencv_highgui opencv_imgproc opencv_videoio opencv_calib3d opencv_cudawarping opencv_cudaobjdetect opencv_cudaarithm opencv_cudabgsegm opencv_cudaimgproc opencv_cudaoptflow opencv_cudastereo opencv_objdetect opencv_xobjdetect opencv_tracking opencv_features2d jetson-inference)

# Dependencies
#find_package(OpenCV REQUIRED)
#find_package(Threads REQUIRED)
#find_package(Boost REQUIRED)

#add_subdirectory(src)

