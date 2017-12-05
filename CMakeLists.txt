cmake_minimum_required(VERSION 3.5)
project(fs CXX)
set(CMAKE_MODULE_PATH )
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(FS_BUILD_TESTS "Build file system tests" ON)

set(CMAKE_CXX_FLAGS "-std=c++17")

add_subdirectory(src)
add_subdirectory(tests)
