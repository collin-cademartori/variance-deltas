cmake_minimum_required(VERSION 3.0)
project(ModelGraphServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boost REQUIRED)
add_executable(graph_test mrf.cpp)
target_link_libraries(graph_test Boost::headers)