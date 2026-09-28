cmake_minimum_required(VERSION 3.16)
project(OpenKAIVisionSnapshotTests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Eigen3 REQUIRED NO_MODULE)
find_package(glog REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenCV 5 REQUIRED COMPONENTS core imgproc calib geometry)
set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")

add_executable(filter_snapshot_test
    filter_snapshot_test.cpp
    ${ROOT}/src/Base/BASE.cpp
    ${ROOT}/src/Base/_ModuleBase.cpp
    ${ROOT}/src/Base/_Thread.cpp
    ${ROOT}/src/Instance/JsonCfg.cpp
    ${ROOT}/src/DataStream/DataStreamBase.cpp
    ${ROOT}/src/DataStream/RGBframe.cpp
    ${ROOT}/src/DataStream/RGBDframe.cpp
    ${ROOT}/src/DataStream/PCLframe.cpp
    ${ROOT}/src/DataStream/IMUstream.cpp
    ${ROOT}/src/Vision/_RGBbase.cpp
    ${ROOT}/src/Vision/RGBD/_RGBDbase.cpp
    ${ROOT}/src/Vision/Pipeline/_ColorConvert.cpp
    ${ROOT}/src/Vision/Pipeline/_Contrast.cpp
    ${ROOT}/src/Vision/Pipeline/_Crop.cpp
    ${ROOT}/src/Vision/Pipeline/_D2RGB.cpp
    ${ROOT}/src/Vision/Pipeline/_Erode.cpp
    ${ROOT}/src/Vision/Pipeline/_HistEqualize.cpp
    ${ROOT}/src/Vision/Pipeline/_InRange.cpp
    ${ROOT}/src/Vision/Pipeline/_Invert.cpp
    ${ROOT}/src/Vision/Pipeline/_Mask.cpp
    ${ROOT}/src/Vision/Pipeline/_Morphology.cpp
    ${ROOT}/src/Vision/Pipeline/_Remap.cpp
    ${ROOT}/src/Vision/Pipeline/_Resize.cpp
    ${ROOT}/src/Vision/Pipeline/_Rotate.cpp
    ${ROOT}/src/Vision/Pipeline/_Thermal2RGB.cpp
    ${ROOT}/src/Vision/Pipeline/_Threshold.cpp)
target_compile_definitions(filter_snapshot_test PRIVATE USE_GLOG USE_OPENCV)
target_compile_options(filter_snapshot_test PRIVATE -UNDEBUG)
target_include_directories(filter_snapshot_test PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(filter_snapshot_test PRIVATE
    Eigen3::Eigen glog::glog Threads::Threads ${OpenCV_LIBS})

enable_testing()
add_test(NAME filter_snapshot_test COMMAND filter_snapshot_test)
set_tests_properties(filter_snapshot_test PROPERTIES TIMEOUT 30)
