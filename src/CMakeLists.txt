cmake_minimum_required(VERSION 3.10)
project(ModelGraphServer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(Boost REQUIRED)
find_package(Eigen3 3.3 REQUIRED NO_MODULE)
find_package(ZLIB REQUIRED)
find_package(OpenSSL REQUIRED)

set(GRAPH_SOURCES mrf.cpp markov.cpp ws_client.cpp read_mrf.cpp read_stan.cpp regression.cpp serialize_tree.cpp)
add_executable(graph_test ${GRAPH_SOURCES})
target_link_libraries(graph_test Boost::headers)
target_link_libraries(graph_test OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries (graph_test Eigen3::Eigen)
target_include_directories(graph_test PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../include)


# add_executable(reg_test reg_test.cpp)
# target_link_libraries (reg_test Eigen3::Eigen)

# add_executable(ws_test ws_test.cpp)
# target_link_libraries(ws_test Boost::headers)
# target_link_libraries(ws_test OpenSSL::SSL OpenSSL::Crypto)
# target_include_directories(ws_test PUBLIC /usr/local/include/simple-websocket-server)
# message(OpenSSL_include_dir=${OPENSSL_INCLUDE_DIR})
# target_include_directories(ws_test PUBLIC ${OPENSSL_INCLUDE_DIR})