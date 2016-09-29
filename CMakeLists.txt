cmake_minimum_required(VERSION 2.8)
set(CMAKE_BUILD_TYPE Release)
project(OpenKAI)
add_definitions("-O3 -w -Wall -c -fmessage-length=0 -MMD -MP -std=c++11")

# Setting specific lib here
link_directories(/usr/local/lib /usr/local/opencv3/lib /home/ubuntu/src/caffe_ssd/caffe/build/lib /home/ubuntu/src/caffe_ssd/caffe/build)

#file(GLOB OpenKAIcpp )
file(GLOB OpenKAIcpp src/*.cpp src/*/*.cpp src/*/*/*.cpp)

add_executable(OpenKAI ${OpenKAIcpp})

# C/C++ settings
include_directories(/home/ubuntu/src/caffe_ssd/caffe/include /home/ubuntu/src/caffe_ssd/caffe/build/src /usr/local/cuda/include /usr/local/include /usr/local/include /usr/local/opencv3/include)
target_link_libraries(OpenKAI pthread boost_system glog caffe opencv_core opencv_imgcodecs opencv_highgui opencv_imgproc opencv_videoio opencv_calib3d opencv_cudawarping opencv_cudaobjdetect opencv_cudaarithm opencv_cudabgsegm opencv_cudaimgproc opencv_cudaoptflow opencv_cudastereo opencv_objdetect opencv_xobjdetect opencv_tracking opencv_features2d)

# Dependencies
#find_package(OpenCV REQUIRED)
#find_package(Threads REQUIRED)
#find_package(Boost REQUIRED)

#add_subdirectory(src)

