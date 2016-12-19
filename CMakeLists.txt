cmake_minimum_required(VERSION 2.8)
set(CMAKE_BUILD_TYPE Release)
project(OpenKAI)
add_definitions("-O3 -w -Wall -c -fmessage-length=0 -MMD -MP -std=c++11")

# Setting specific lib here
link_directories(/home/ubuntu/src/jetson-inference/build/aarch64/lib /usr/local/lib /usr/lib)

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
include_directories(/home/ubuntu/src/jetson-inference/cuda /home/ubuntu/src/jetson-inference/build/aarch64/include /usr/local/cuda/include /usr/local/include /usr/include)

target_link_libraries(OpenKAI pthread boost_system glog opencv_core opencv_highgui opencv_imgproc opencv_calib3d opencv_objdetect opencv_features2d opencv_gpu opencv_superres jetson-inference)

# Dependencies
#find_package(OpenCV REQUIRED)
#find_package(Threads REQUIRED)
#find_package(Boost REQUIRED)

#add_subdirectory(src)

