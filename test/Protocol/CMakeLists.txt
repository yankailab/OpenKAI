cmake_minimum_required(VERSION 3.18)
project(OpenKAIProtocolTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
find_package(Eigen3 5 REQUIRED CONFIG)
find_package(glog REQUIRED)
find_package(Threads REQUIRED)
find_package(Curses REQUIRED)
find_package(OpenSSL REQUIRED)
enable_testing()

get_filename_component(OPENKAI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
add_executable(ws_console_shutdown
    ws_console_shutdown.cpp
    ${OPENKAI_ROOT}/src/Protocol/_WSconsole.cpp
    ${OPENKAI_ROOT}/src/Protocol/_JSONbase.cpp
    ${OPENKAI_ROOT}/src/Protocol/_ProtocolBase.cpp
    ${OPENKAI_ROOT}/src/IO/_IObase.cpp
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
    ${OPENKAI_ROOT}/src/DataObject/SharedMemoryFrame.cpp
    ${OPENKAI_ROOT}/src/DataObject/UGLIDcellStream.cpp
)
target_include_directories(ws_console_shutdown PRIVATE ${OPENKAI_ROOT}/src ${CURSES_INCLUDE_DIRS})
target_compile_definitions(ws_console_shutdown PRIVATE USE_GLOG)
target_link_libraries(ws_console_shutdown PRIVATE Eigen3::Eigen glog::glog Threads::Threads ${CURSES_LIBRARIES} OpenSSL::Crypto rt)
add_test(NAME ws_console_shutdown COMMAND ws_console_shutdown)
set_tests_properties(ws_console_shutdown PROPERTIES TIMEOUT 15)
