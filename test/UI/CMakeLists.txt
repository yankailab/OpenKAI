cmake_minimum_required(VERSION 3.16)
project(OpenKAIImageUITests LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(Eigen3 REQUIRED NO_MODULE)
find_package(glog REQUIRED)
find_package(Threads REQUIRED)
find_package(OpenCV 5 REQUIRED COMPONENTS core imgproc highgui videoio)
set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")

add_executable(image_input_test
    image_input_test.cpp
    ${ROOT}/src/Base/BASE.cpp
    ${ROOT}/src/Base/_ModuleBase.cpp
    ${ROOT}/src/Base/_Thread.cpp
    ${ROOT}/src/Instance/JsonCfg.cpp
    ${ROOT}/src/DataStream/DataStreamBase.cpp
    ${ROOT}/src/DataStream/RGBframe.cpp
    ${ROOT}/src/UI/_UIbase.cpp
    ${ROOT}/src/UI/_WindowCV.cpp
    ${ROOT}/src/UI/_GstOutput.cpp)
target_compile_definitions(image_input_test PRIVATE USE_GLOG USE_OPENCV)
target_compile_options(image_input_test PRIVATE -UNDEBUG)
target_include_directories(image_input_test PRIVATE ${OpenCV_INCLUDE_DIRS})
target_link_libraries(image_input_test PRIVATE
    Eigen3::Eigen glog::glog Threads::Threads ${OpenCV_LIBS})

enable_testing()
add_test(NAME image_input_test COMMAND image_input_test)
set_tests_properties(image_input_test PROPERTIES TIMEOUT 30)
