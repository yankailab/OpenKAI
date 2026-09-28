cmake_minimum_required(VERSION 3.16)
project(OpenKAIGeometryStreamTests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Eigen3 REQUIRED NO_MODULE)
find_package(glog REQUIRED)
find_package(Threads REQUIRED)
set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
add_executable(geometry_stream_test
    geometry_stream_test.cpp
    ${ROOT}/src/Base/BASE.cpp
    ${ROOT}/src/Base/_ModuleBase.cpp
    ${ROOT}/src/Base/_Thread.cpp
    ${ROOT}/src/Instance/JsonCfg.cpp
    ${ROOT}/src/IO/_IObase.cpp
    ${ROOT}/src/Universe/_ReferenceFrame.cpp
    ${ROOT}/src/Universe/Geometry/_GeometryBase.cpp
    ${ROOT}/src/Universe/Geometry/PointCloud/_PointCloud.cpp
    ${ROOT}/src/Universe/Geometry/PointCloud/_PCfile.cpp
    ${ROOT}/src/Universe/Geometry/PointCloud/Pipeline/_PCtransform.cpp
    ${ROOT}/src/Universe/Geometry/PointCloud/Pipeline/_PCmerge.cpp
    ${ROOT}/src/Universe/Geometry/PointCloud/Pipeline/_PCsend.cpp
    ${ROOT}/src/Universe/Geometry/PointCloud/Pipeline/_PCrecv.cpp
    ${ROOT}/src/DataStream/DataStreamBase.cpp
    ${ROOT}/src/DataStream/PCLframe.cpp)
target_compile_definitions(geometry_stream_test PRIVATE USE_GLOG)
target_compile_options(geometry_stream_test PRIVATE -UNDEBUG)
target_link_libraries(geometry_stream_test PRIVATE Eigen3::Eigen glog::glog Threads::Threads)
enable_testing()
add_test(NAME geometry_stream_test COMMAND geometry_stream_test)
set_tests_properties(geometry_stream_test PROPERTIES TIMEOUT 30)

find_package(OpenCV REQUIRED COMPONENTS core)
add_executable(grid_lidar_stream_test
    grid_lidar_stream_test.cpp
    ${ROOT}/src/Base/BASE.cpp
    ${ROOT}/src/Base/_ModuleBase.cpp
    ${ROOT}/src/Base/_Thread.cpp
    ${ROOT}/src/Instance/JsonCfg.cpp
    ${ROOT}/src/Universe/_ReferenceFrame.cpp
    ${ROOT}/src/Universe/Geometry/_GeometryBase.cpp
    ${ROOT}/src/Universe/Geometry/PointCloud/_PointCloud.cpp
    ${ROOT}/src/Universe/Grid/_OctreeBase.cpp
    ${ROOT}/src/Universe/Grid/_OctreeGrid.cpp
    ${ROOT}/src/DataStream/DataStreamBase.cpp
    ${ROOT}/src/DataStream/PCLframe.cpp
    ${ROOT}/src/DataStream/IMUstream.cpp
    ${ROOT}/src/Sensor/LiDAR/_Livox2.cpp
    ${ROOT}/src/Dependencies/SensorFusion/SensorFusion.cpp)
target_compile_definitions(grid_lidar_stream_test PRIVATE USE_GLOG USE_OPENCV)
target_compile_options(grid_lidar_stream_test PRIVATE -UNDEBUG)
target_include_directories(grid_lidar_stream_test PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(grid_lidar_stream_test PRIVATE Eigen3::Eigen glog::glog Threads::Threads ${OpenCV_LIBS})
add_test(NAME grid_lidar_stream_test COMMAND grid_lidar_stream_test)
set_tests_properties(grid_lidar_stream_test PROPERTIES TIMEOUT 30)

add_executable(slam_stream_test
    slam_stream_test.cpp
    ${ROOT}/src/SLAM/_SLAMbase.cpp
    ${ROOT}/src/Navigation/_NavBase.cpp
    ${ROOT}/src/Universe/_ReferenceFrame.cpp
    ${ROOT}/src/Base/BASE.cpp
    ${ROOT}/src/Base/_ModuleBase.cpp
    ${ROOT}/src/Base/_Thread.cpp
    ${ROOT}/src/Instance/JsonCfg.cpp
    ${ROOT}/src/DataStream/DataStreamBase.cpp
    ${ROOT}/src/DataStream/PCLframe.cpp
    ${ROOT}/src/DataStream/IMUstream.cpp)
target_compile_definitions(slam_stream_test PRIVATE USE_GLOG)
target_compile_options(slam_stream_test PRIVATE -UNDEBUG)
target_link_libraries(slam_stream_test PRIVATE Eigen3::Eigen glog::glog Threads::Threads)
add_test(NAME slam_stream_test COMMAND slam_stream_test)
set_tests_properties(slam_stream_test PROPERTIES TIMEOUT 30)
