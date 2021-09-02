cmake_minimum_required(VERSION 3.18)
set(CMAKE_BUILD_TYPE Debug)
project(OpenKAI)

LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

OPTION(USE_CAFFE "Use Caffe" OFF)
OPTION(USE_CUDA "Use NVIDIA CUDA" OFF)
OPTION(USE_CHILITAGS "Use Chilitags" OFF)
OPTION(USE_DARKNET "Use Darknet" OFF)
OPTION(USE_DYNAMIXEL "Use Dynamixel" OFF)
OPTION(USE_GUI "Use GUI" OFF)
OPTION(USE_JETSON_INFERENCE "Use NVIDIA TensorRT" OFF)
OPTION(USE_LIVOX "Use Livox" OFF)
OPTION(USE_MYNTEYE "Use Mynteye" OFF)
OPTION(USE_GUI "Use GUI" OFF)
OPTION(USE_OPEN3D "Use Open3D" OFF)
OPTION(USE_OPENCL "Use OpenCL" OFF)
OPTION(USE_OPENCV "Use OpenCV" ON)
OPTION(USE_OPENCV_CONTRIB "Use OpenCV contribution" OFF)
OPTION(USE_ORB_SLAM "Use USE_ORB_SLAM" OFF)
OPTION(USE_OCR "Use Tesseract" OFF)
OPTION(USE_OPENALPR "Use OpenALPR" OFF)
OPTION(USE_PYLON "Use Basler camera with Pylon" OFF)
OPTION(USE_REALSENSE "Use Intel RealSense" OFF)
OPTION(USE_XARM "Use xArm" OFF)

OPTION(WITH_ACTUATOR "" OFF)
OPTION(WITH_APP_3DSCAN "" OFF)
OPTION(WITH_APP_AUTOPILOT "" OFF)
OPTION(WITH_APP_BENCHROVER "" OFF)
OPTION(WITH_APP_CAMCALIB "" OFF)
OPTION(WITH_APP_DRONEBOX "" OFF)
OPTION(WITH_APP_KUPHENOROVER "" OFF)
OPTION(WITH_APP_MEASUREMENT "" OFF)
OPTION(WITH_APP_PCSCAN "" OFF)
OPTION(WITH_APP_ROBOTARM "" OFF)
OPTION(WITH_APP_SURVEILLENCE "" OFF)
OPTION(WITH_APP_TAGROVER "" OFF)
OPTION(WITH_APP_UTPHENOROVER "" OFF)
OPTION(WITH_COMPUTE "" OFF)
OPTION(WITH_CONTROL "" OFF)
OPTION(WITH_DATA "" OFF)
OPTION(WITH_DETECTOR "" OFF)
OPTION(WITH_DNN "" OFF)
OPTION(WITH_FILTER "" OFF)
OPTION(WITH_GCS "" OFF)
OPTION(WITH_IO "" OFF)
OPTION(WITH_NAVIGATION "" OFF)
OPTION(WITH_POINTCLOUD "" OFF)
OPTION(WITH_PROTOCOL "" OFF)
OPTION(WITH_REGRESSION "" OFF)
OPTION(WITH_SENSOR "" OFF)
OPTION(WITH_SLAM "" OFF)
OPTION(WITH_STATE "" OFF)
OPTION(WITH_TRACKER "" OFF)
OPTION(WITH_UI "" OFF)
OPTION(WITH_UNIVERSE "" OFF)
OPTION(WITH_VISION "" OFF)

find_package(Eigen3 3.1.0 REQUIRED)

#set(OpenKAI_definitions "-O3 -w -Wall -c -fmessage-length=0 -MMD -MP -std=c++17")
set(OpenKAI_definitions "-w -Wall -c -fmessage-length=0 -MMD -MP -std=c++17")
set(OpenKAI_libDir /usr/lib /usr/local/lib)
set(OpenKAI_include /usr/local/include /usr/include ${EIGEN3_INCLUDE_DIR})
set(OpenKAI_lib pthread boost_system glog uuid ${EIGEN3_LIBS} gsl gslcblas ncurses ssl crypto)

# File
#file(GLOB_RECURSE OpenKAI_cpp CONFIGURE_DEPENDS src/*.cpp src/*.cc src/*.c)
file(GLOB OpenKAI_cpp src/Base/*.cpp src/Base/*.cc src/Base/*.c)

file(GLOB Arithmetic_cpp src/Arithmetic/*.cpp src/Arithmetic/*.cc src/Arithmetic/*.c)
set(OpenKAI_cpp ${OpenKAI_cpp} ${Arithmetic_cpp})

file(GLOB Config_cpp src/Config/*.cpp src/Config/*.cc src/Config/*.c)
set(OpenKAI_cpp ${OpenKAI_cpp} ${Config_cpp})

file(GLOB Dependency_cpp src/Dependency/*.cpp src/Dependency/*.cc src/Dependency/*.c)
set(OpenKAI_cpp ${OpenKAI_cpp} ${Dependency_cpp})

file(GLOB Primitive_cpp src/Primitive/*.cpp src/Primitive/*.cc src/Primitive/*.c)
set(OpenKAI_cpp ${OpenKAI_cpp} ${Primitive_cpp})

file(GLOB Script_cpp src/Script/*.cpp src/Script/*.cc src/Script/*.c)
set(OpenKAI_cpp ${OpenKAI_cpp} ${Script_cpp})

file(GLOB Startup_cpp src/Startup/*.cpp src/Startup/*.cc src/Startup/*.c)
set(OpenKAI_cpp ${OpenKAI_cpp} ${Startup_cpp})


if(WITH_ACTUATOR)
file(GLOB Actuator_cpp src/Actuator/*.cpp src/Actuator/*.cc src/Actuator/*.c)
set(OpenKAI_cpp ${OpenKAI_cpp} ${Actuator_cpp})
endif()



# Dependencies
if(USE_CHILITAGS)
  add_definitions(-DUSE_CHILITAGS)
  set(OpenKAI_lib ${OpenKAI_lib} chilitags)
endif()

if(USE_OPEN3D)
  add_definitions(-DUSE_OPEN3D)
  find_package(Open3D REQUIRED)
  set(OpenKAI_include ${OpenKAI_include} ${Open3D_INCLUDE_DIRS})
  set(OpenKAI_lib ${OpenKAI_lib} ${Open3D_LIBRARIES})
  set(OpenKAI_libDir ${OpenKAI_libDir} ${Open3D_LIBRARY_DIRS})
  set(Open3D_jsoncpp "Open3D/3rdparty/jsoncpp/include" CACHE STRING "")
  set(OpenKAI_include ${OpenKAI_include} ${Open3D_jsoncpp})
endif()

if(USE_OPENCL)
  add_definitions(-DUSE_OPENCL)
  find_package(OpenCL REQUIRED)
  set(OpenKAI_include ${OpenKAI_include} ${OpenCL_INCLUDE_DIRS})
  set(OpenKAI_lib ${OpenKAI_lib} ${OpenCL_LIBRARIES})
endif()

if(USE_OPENCV)
  add_definitions(-DUSE_OPENCV)
  set(OpenCV_include "/usr/local/include/opencv4" CACHE STRING "")
  set(OpenCV_lib "/usr/local/lib" CACHE STRING "")
  set(OpenKAI_include ${OpenKAI_include} ${OpenCV_include})
  set(OpenKAI_libDir ${OpenKAI_libDir} ${OpenCV_lib})
  set(OpenKAI_lib ${OpenKAI_lib} opencv_core opencv_imgcodecs opencv_highgui opencv_imgproc opencv_videoio opencv_calib3d opencv_objdetect opencv_features2d opencv_dnn opencv_freetype)
endif()

if(USE_OPENCV_CONTRIB)
  add_definitions(-DUSE_OPENCV_CONTRIB)
  set(OpenKAI_lib ${OpenKAI_lib} opencv_xobjdetect opencv_tracking opencv_aruco)
endif()

if(USE_CUDA)
  add_definitions(-DUSE_CUDA)
  set(CUDA_include "/usr/local/cuda/include" CACHE STRING "")
  set(OpenKAI_include ${OpenKAI_include} ${CUDA_include})
  set(CUDA_ARCH "72" CACHE STRING "")
  set(OpenKAI_lib ${OpenKAI_lib} opencv_cudawarping opencv_cudaobjdetect opencv_cudaarithm opencv_cudabgsegm opencv_cudaimgproc opencv_cudaoptflow opencv_cudastereo)
endif()

if(USE_DYNAMIXEL)
  add_definitions(-DUSE_DYNAMIXEL)
  set(OpenKAI_lib ${OpenKAI_lib} dxl_x64_cpp)
endif()

if(USE_JETSON_INFERENCE)
  add_definitions(-DUSE_JETSON_INFERENCE)
  set(OpenKAI_include ${OpenKAI_include} "/usr/local/include/jetson-inference")
  set(OpenKAI_lib ${OpenKAI_lib} jetson-inference)
endif()

if(USE_MYNTEYE)
  add_definitions(-DUSE_MYNTEYE)
  set(OpenKAI_lib ${OpenKAI_lib} mynteye_depth)
endif()

if(USE_REALSENSE)
  add_definitions(-DUSE_REALSENSE)
  set(OpenKAI_lib ${OpenKAI_lib} realsense2)
endif()

if(USE_XARM)
  add_definitions(-DUSE_XARM)
  set(XARM_include "/home/kai/dev/xArm-CPLUS-SDK/include" CACHE STRING "")
  set(OpenKAI_include ${OpenKAI_include} ${XARM_include})
  set(OpenKAI_lib ${OpenKAI_lib} xarm)
endif()

if(USE_DARKNET)
  add_definitions(-DUSE_DARKNET)  
  set(Darknet_root "/darknet" CACHE STRING "")
  set(OpenKAI_libDir ${OpenKAI_libDir} ${Darknet_root})
  set(OpenKAI_include ${OpenKAI_include} "${Darknet_root}/api")
  set(OpenKAI_lib ${OpenKAI_lib} darknet)
endif()

if(USE_CAFFE)
  add_definitions(-DUSE_CAFFE)
  set(Caffe_distribute "/caffe/distribute" CACHE STRING "")
  set(OpenKAI_include ${OpenKAI_include} "${Caffe_distribute}/include")
  set(OpenKAI_libDir ${OpenKAI_libDir} "${Caffe_distribute}/lib")
  set(OpenKAI_lib ${OpenKAI_lib} caffe)
endif()

if(USE_PYLON)
  add_definitions(-DUSE_PYLON)
  set(Pylon_root "/opt/pylon5" CACHE STRING "")
  set(OpenKAI_include ${OpenKAI_include} "${Pylon_root}/include")
  set(OpenKAI_libDir ${OpenKAI_libDir} "${Pylon_root}/lib64")
  set(OpenKAI_lib ${OpenKAI_lib} Log_gcc_v3_0_Basler_pylon_v5_0 MathParser_gcc_v3_0_Basler_pylon_v5_0 XmlParser_gcc_v3_0_Basler_pylon_v5_0 NodeMapData_gcc_v3_0_Basler_pylon_v5_0 pylonc pylonbase pylonutility GenApi_gcc_v3_0_Basler_pylon_v5_0 GCBase_gcc_v3_0_Basler_pylon_v5_0)
endif()

if(USE_ORB_SLAM)
  find_package(Pangolin REQUIRED)
  add_definitions(-DUSE_ORB_SLAM)
  set(ORB_SLAM_root "/ORB_SLAM3" CACHE STRING "")
  set(OpenKAI_include ${OpenKAI_include} ${Pangolin_INCLUDE_DIRS})
  set(OpenKAI_include ${OpenKAI_include} ${ORB_SLAM_root})
  set(OpenKAI_include ${OpenKAI_include} "${ORB_SLAM_root}/include")
  set(OpenKAI_libDir ${OpenKAI_libDir} "${ORB_SLAM_root}/lib")
  set(OpenKAI_lib ${OpenKAI_lib} ${Pangolin_LIBRARIES} ORB_SLAM3)
endif()

if(USE_OCR)
  add_definitions(-DUSE_OCR)
  set(OpenKAI_lib ${OpenKAI_lib} tesseract)
endif()

if(USE_OPENALPR)
  add_definitions(-DUSE_OPENALPR)
  set(OpenKAI_lib ${OpenKAI_lib} openalpr)
endif()

if(USE_LIVOX)
  add_definitions(-DUSE_LIVOX)
  set(OpenKAI_lib ${OpenKAI_lib} livox_sdk_static)
endif()

if(USE_GUI)
  add_definitions(-DUSE_GUI)
  set(OpenKAI_lib ${OpenKAI_lib} glfw GL m GLU GLEW)
endif()


# Setting specific lib here
link_directories(${OpenKAI_libDir})

if(USE_CUDA)
find_package(CUDA)
set(
	CUDA_NVCC_FLAGS
	${CUDA_NVCC_FLAGS}; 
    -O3 -gencode arch=compute_${CUDA_ARCH},code=sm_${CUDA_ARCH}
)
endif()

# Definitions
add_definitions(${OpenKAI_definitions})

# Exe
if(USE_CUDA)
cuda_add_executable(OpenKAI ${OpenKAI_cpp})
else()
add_executable(OpenKAI ${OpenKAI_cpp})
endif()

# C/C++ settings
include_directories(${OpenKAI_include})
target_link_libraries(OpenKAI ${OpenKAI_lib})
