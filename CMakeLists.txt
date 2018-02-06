cmake_minimum_required(VERSION 2.8)
set(CMAKE_BUILD_TYPE Release)
project(OpenKAI)

add_definitions("-O3 -w -Wall -c -fmessage-length=0 -MMD -MP -std=c++11")

LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

OPTION(USE_CUDA "Use NVIDIA CUDA" ON)
OPTION(USE_OPENCV4TEGRA "Use NVIDIA OpenCV4Tegra for JetsonTX1" ON)
OPTION(USE_TENSORRT "Use NVIDIA TensorRT" ON)
OPTION(USE_SSD "Use Caffe SSD branch" OFF)
OPTION(USE_CAFFE "Use Caffe" OFF)
OPTION(USE_ZED "Use Stereolabs ZED camera" OFF)
OPTION(USE_ORB_SLAM2 "Use Orb_Slam2" OFF)

find_package(Eigen3 3.1.0 REQUIRED)

set(CUDA_include "/usr/local/cuda/include" CACHE STRING "")
set(OpenKAI_libDir /usr/lib /usr/local/lib)
set(OpenKAI_include /usr/local/include /usr/include ${EIGEN3_INCLUDE_DIR})
set(OpenKAI_lib pthread boost_system glog uuid ${EIGEN3_LIBS})

if(USE_CUDA)
  add_definitions(-DUSE_CUDA)
  set(OpenKAI_include ${OpenKAI_include} ${CUDA_include})
  set(CUDA_ARCH "62" CACHE STRING "")
endif()

if(USE_OPENCV4TEGRA)
  add_definitions(-DUSE_OPENCV4TEGRA)
  set(OpenKAI_lib ${OpenKAI_lib} opencv_core opencv_highgui opencv_imgproc opencv_calib3d opencv_objdetect opencv_features2d opencv_gpu opencv_superres)
else()
  set(OpenCV_include "/usr/local/include" CACHE STRING "")
  set(OpenCV_lib "/usr/local/lib" CACHE STRING "")
  set(OpenKAI_libDir ${OpenKAI_libDir} ${OpenCV_lib})
  set(OpenKAI_include ${OpenKAI_include} ${OpenCV_include})
  set(OpenKAI_lib ${OpenKAI_lib} opencv_core opencv_imgcodecs opencv_highgui opencv_imgproc opencv_videoio opencv_calib3d opencv_cudawarping opencv_cudaobjdetect opencv_cudaarithm opencv_cudabgsegm opencv_cudaimgproc opencv_cudaoptflow opencv_cudastereo opencv_objdetect opencv_xobjdetect opencv_tracking opencv_features2d)
endif()

if(USE_TENSORRT)
  add_definitions(-DUSE_TENSORRT)  
  set(TensorRT_build /jetson-inference-batch/build/${CMAKE_SYSTEM_PROCESSOR} CACHE STRING "")
  set(OpenKAI_libDir ${OpenKAI_libDir} "${TensorRT_build}/lib")
  set(OpenKAI_include ${OpenKAI_include} "${TensorRT_build}/include")
  set(OpenKAI_lib ${OpenKAI_lib} jetson-inference)
endif()

if(USE_SSD)
  add_definitions(-DUSE_SSD)
  set(SSD_include "/caffe/include /caffe/build/src" CACHE STRING "")
  set(SSD_lib "/caffe/build/lib /caffe/build" CACHE STRING "")
  set(OpenKAI_libDir ${OpenKAI_libDir} ${SSD_lib})
  set(OpenKAI_include ${OpenKAI_include} ${SSD_include})
  set(OpenKAI_lib ${OpenKAI_lib} caffe)
elseif(USE_CAFFE)
  add_definitions(-DUSE_CAFFE)
  set(Caffe_include "/caffe/include /caffe/build/src" CACHE STRING "")
  set(Caffe_lib "/caffe/build/lib /caffe/build" CACHE STRING "")
  set(OpenKAI_libDir ${OpenKAI_libDir} ${Caffe_lib})
  set(OpenKAI_include ${OpenKAI_include} ${Caffe_include})
  set(OpenKAI_lib ${OpenKAI_lib} caffe)
endif()

if(USE_ZED)
  add_definitions(-DUSE_ZED)
  set(ZED_root "/usr/local/zed" CACHE STRING "")
  set(OpenKAI_libDir ${OpenKAI_libDir} "${ZED_root}/lib")
  set(OpenKAI_include ${OpenKAI_include} "${ZED_root}/include")
  set(OpenKAI_lib ${OpenKAI_lib} sl_zed sl_core sl_depthcore sl_tracking)
endif()

if(USE_ORB_SLAM2)
  find_package(Pangolin REQUIRED)
  add_definitions(-DUSE_ORB_SLAM2)
  set(ORB_SLAM2_root "/orb_slam2_gpu" CACHE STRING "")
  set(OpenKAI_include ${OpenKAI_include} "${ORB_SLAM2_root} ${ORB_SLAM2_root}/include ${Pangolin_INCLUDE_DIRS}")
  set(OpenKAI_libDir ${OpenKAI_libDir} "${ORB_SLAM2_root}/build/lib")
  set(OpenKAI_lib ${OpenKAI_lib} ${Pangolin_LIBRARIES} ORB_SLAM2)
endif()

# Setting specific lib here
link_directories(${OpenKAI_libDir})

# setup CUDA
find_package(CUDA)

set(
	CUDA_NVCC_FLAGS
	${CUDA_NVCC_FLAGS}; 
    -O3 -gencode arch=compute_${CUDA_ARCH},code=sm_${CUDA_ARCH}
)

# File
file(GLOB OpenKAI_cpp src/*.cpp src/*/*.cpp src/*/*/*.cpp src/*/*/*/*.cpp src/*/*/*/*/*.cpp src/*/*/*/*/*/*.cpp src/*/*/*/*/*/*/*.cpp src/*.c src/*/*.c src/*/*/*.c src/*/*/*/*.c src/*/*/*/*/*.c src/*/*/*/*/*/*.c src/*/*/*/*/*/*/*.c)

cuda_add_executable(OpenKAI ${OpenKAI_cpp})

# C/C++ settings
include_directories(${OpenKAI_include})
target_link_libraries(OpenKAI ${OpenKAI_lib})

