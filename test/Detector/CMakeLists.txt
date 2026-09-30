cmake_minimum_required(VERSION 3.18)
project(OpenKAIDetectorTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Eigen3 5 REQUIRED CONFIG)
find_package(OpenCV REQUIRED)
find_package(glog REQUIRED)
find_package(Threads REQUIRED)
find_package(Curses REQUIRED)
find_path(ONNXRUNTIME_INCLUDE_DIR onnxruntime_cxx_api.h
    PATH_SUFFIXES onnxruntime onnxruntime/core/session REQUIRED)
find_library(ONNXRUNTIME_LIBRARY onnxruntime REQUIRED)
enable_testing()

get_filename_component(OPENKAI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
add_library(depth_detector_support STATIC
    ${OPENKAI_ROOT}/src/Detector/_YOLO26depthEstONNX.cpp
    ${OPENKAI_ROOT}/src/Detector/_DetectorBase.cpp
    ${OPENKAI_ROOT}/src/Base/BASE.cpp
    ${OPENKAI_ROOT}/src/Base/_ModuleBase.cpp
    ${OPENKAI_ROOT}/src/Base/_Thread.cpp
    ${OPENKAI_ROOT}/src/Instance/InstanceMgr.cpp
    ${OPENKAI_ROOT}/src/Instance/JsonCfg.cpp
    ${OPENKAI_ROOT}/src/Instance/Modules.cpp
    ${OPENKAI_ROOT}/src/Instance/DataObjects.cpp
    ${OPENKAI_ROOT}/src/UI/_Console.cpp
    ${OPENKAI_ROOT}/src/DataObject/DataObjBase.cpp
    ${OPENKAI_ROOT}/src/DataObject/BBoxStream.cpp
    ${OPENKAI_ROOT}/src/DataObject/BytePacket.cpp
    ${OPENKAI_ROOT}/src/DataObject/IMUstream.cpp
    ${OPENKAI_ROOT}/src/DataObject/LineFrame.cpp
    ${OPENKAI_ROOT}/src/DataObject/PCLframe.cpp
    ${OPENKAI_ROOT}/src/DataObject/PCLmap.cpp
    ${OPENKAI_ROOT}/src/DataObject/RGBframe.cpp
    ${OPENKAI_ROOT}/src/DataObject/RGBDframe.cpp
    ${OPENKAI_ROOT}/src/DataObject/SharedMemoryFrame.cpp
    ${OPENKAI_ROOT}/src/DataObject/UGLIDcellStream.cpp
)
target_include_directories(depth_detector_support PUBLIC
    ${OPENKAI_ROOT}/src ${CURSES_INCLUDE_DIRS} ${OpenCV_INCLUDE_DIRS}
    ${ONNXRUNTIME_INCLUDE_DIR})
target_compile_definitions(depth_detector_support PUBLIC USE_GLOG USE_OPENCV USE_ONNXRUNTIME)
target_link_libraries(depth_detector_support PUBLIC Eigen3::Eigen glog::glog Threads::Threads
    ${CURSES_LIBRARIES} ${OpenCV_LIBS} ${ONNXRUNTIME_LIBRARY} rt)
add_executable(yolo26_depth yolo26_depth.cpp)
target_link_libraries(yolo26_depth PRIVATE depth_detector_support)
add_executable(depth_benchmark depth_benchmark.cpp)
target_link_libraries(depth_benchmark PRIVATE depth_detector_support)
add_test(NAME yolo26_depth
    COMMAND yolo26_depth ${CMAKE_CURRENT_SOURCE_DIR}/fixtures)
