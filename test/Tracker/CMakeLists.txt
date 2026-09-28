cmake_minimum_required(VERSION 3.16)
project(OpenKAITrackerStreamTests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Eigen3 REQUIRED NO_MODULE)
find_package(glog REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenCV REQUIRED COMPONENTS core imgproc tracking)
set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
add_executable(tracker_stream_test
    streamInput.cpp
    ${ROOT}/src/Tracker/_TrackerBase.cpp
    ${ROOT}/src/Base/BASE.cpp
    ${ROOT}/src/Base/_ModuleBase.cpp
    ${ROOT}/src/Base/_Thread.cpp
    ${ROOT}/src/Instance/JsonCfg.cpp
    ${ROOT}/src/DataStream/DataStreamBase.cpp
    ${ROOT}/src/DataStream/RGBframe.cpp)
target_compile_definitions(tracker_stream_test PRIVATE USE_GLOG USE_OPENCV)
target_compile_options(tracker_stream_test PRIVATE -UNDEBUG)
target_include_directories(tracker_stream_test PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(tracker_stream_test PRIVATE Eigen3::Eigen glog::glog Threads::Threads ${OpenCV_LIBS})
enable_testing()
add_test(NAME tracker_stream_test COMMAND tracker_stream_test)
set_tests_properties(tracker_stream_test PROPERTIES TIMEOUT 30)
