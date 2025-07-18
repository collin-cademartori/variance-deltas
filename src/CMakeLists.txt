cmake_minimum_required(VERSION 3.0)
project(ModelGraphServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boost REQUIRED)
find_package (Eigen3 3.3 REQUIRED NO_MODULE)

add_executable(graph_test mrf.cpp)
target_link_libraries(graph_test Boost::headers)
target_link_libraries (graph_test Eigen3::Eigen)

add_executable(reg_test reg_test.cpp)
target_link_libraries (reg_test Eigen3::Eigen)