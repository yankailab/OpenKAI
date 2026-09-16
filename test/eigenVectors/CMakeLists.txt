cmake_minimum_required(VERSION 3.18)
project(EigenVectorTests LANGUAGES CXX)

find_package(Eigen3 5 REQUIRED CONFIG)
find_package(glog REQUIRED)

add_executable(eigen_vectors vectors.cpp)
target_compile_features(eigen_vectors PRIVATE cxx_std_17)
target_compile_definitions(eigen_vectors PRIVATE USE_GLOG)
target_compile_options(eigen_vectors PRIVATE -UNDEBUG)
target_link_libraries(eigen_vectors PRIVATE Eigen3::Eigen glog::glog)

enable_testing()
add_test(NAME eigen_vectors COMMAND eigen_vectors)
