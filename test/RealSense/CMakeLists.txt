cmake_minimum_required(VERSION 3.18)
project(OpenKAIRealSenseTests LANGUAGES CXX)

find_package(realsense2 2.58.4 REQUIRED CONFIG)
find_package(Eigen3 5 REQUIRED CONFIG)
find_package(Threads REQUIRED)
option(ENABLE_UBSAN "Check SDK option values and IMU preview fusion with UndefinedBehaviorSanitizer" OFF)
enable_testing()

add_executable(realsense_options options.cpp)
target_compile_features(realsense_options PRIVATE cxx_std_17)
target_compile_options(realsense_options PRIVATE -Wall -Wextra)
target_link_libraries(realsense_options PRIVATE realsense2::realsense2)
if(ENABLE_UBSAN AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(realsense_options PRIVATE -fsanitize=undefined -fno-sanitize-recover=all)
    target_link_options(realsense_options PRIVATE -fsanitize=undefined -fno-sanitize-recover=all)
endif()

# Uses only the SDK software device and processing blocks; never opens a camera.
add_test(NAME realsense_options COMMAND realsense_options)

add_executable(realsense_imu_preview
    imu_preview.cpp
    ../../src/Dependencies/SensorFusion/SensorFusion.cpp)
target_compile_features(realsense_imu_preview PRIVATE cxx_std_17)
target_compile_options(realsense_imu_preview PRIVATE -Wall -Wextra)
target_link_libraries(realsense_imu_preview PRIVATE Eigen3::Eigen Threads::Threads)
if(ENABLE_UBSAN AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(realsense_imu_preview PRIVATE -fsanitize=undefined -fno-sanitize-recover=all)
    target_link_options(realsense_imu_preview PRIVATE -fsanitize=undefined -fno-sanitize-recover=all)
endif()
add_test(NAME realsense_imu_preview COMMAND realsense_imu_preview)
