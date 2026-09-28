cmake_minimum_required(VERSION 3.16)
project(OpenKAIDataStreamTests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Eigen3 REQUIRED NO_MODULE)
find_package(glog REQUIRED)
find_package(Threads REQUIRED)
set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
add_library(stream_test_support STATIC
    consoleStub.cpp
    ${ROOT}/src/Base/BASE.cpp
    ${ROOT}/src/Instance/JsonCfg.cpp
    ${ROOT}/src/DataStream/DataStreamBase.cpp
    ${ROOT}/src/DataStream/PCLframe.cpp
    ${ROOT}/src/DataStream/LineFrame.cpp
    ${ROOT}/src/DataStream/PCLmap.cpp
    ${ROOT}/src/DataStream/IMUstream.cpp)
target_compile_definitions(stream_test_support PUBLIC USE_GLOG)
target_link_libraries(stream_test_support PUBLIC Eigen3::Eigen glog::glog Threads::Threads)
add_executable(stream_snapshot_test streamSnapshot.cpp)
add_executable(viewer_snapshot_test viewerSnapshot.cpp)
foreach(target stream_snapshot_test viewer_snapshot_test)
    target_compile_options(${target} PRIVATE -UNDEBUG)
    target_link_libraries(${target} PRIVATE stream_test_support)
endforeach()
enable_testing()
add_test(NAME stream_snapshot_test COMMAND stream_snapshot_test)
add_test(NAME viewer_snapshot_test COMMAND viewer_snapshot_test)
set_tests_properties(stream_snapshot_test viewer_snapshot_test PROPERTIES TIMEOUT 30)

find_package(OpenCV REQUIRED COMPONENTS core)
add_executable(image_snapshot_test
    imageSnapshot.cpp
    ${ROOT}/src/DataStream/RGBframe.cpp
    ${ROOT}/src/DataStream/RGBDframe.cpp)
target_compile_definitions(image_snapshot_test PRIVATE USE_OPENCV)
target_compile_options(image_snapshot_test PRIVATE -UNDEBUG)
target_include_directories(image_snapshot_test PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(image_snapshot_test PRIVATE stream_test_support ${OpenCV_LIBS})
add_test(NAME image_snapshot_test COMMAND image_snapshot_test)
set_tests_properties(image_snapshot_test PROPERTIES TIMEOUT 30)
