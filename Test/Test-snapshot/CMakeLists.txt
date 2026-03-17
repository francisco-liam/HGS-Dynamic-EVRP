cmake_minimum_required(VERSION 3.15)
project(HGS_Test_snapshot)
set(CMAKE_CXX_STANDARD 17)

add_executable(snapshot_test test_snapshot.cpp)
target_link_libraries(snapshot_test lib_static)
