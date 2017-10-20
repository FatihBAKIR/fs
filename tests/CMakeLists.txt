cmake_minimum_required(VERSION 3.5)
project(fs CXX)

add_executable(cont_file cont_file_tests.cpp)
target_link_libraries(cont_file PRIVATE fs_cont_data fs_ram_blk)

enable_testing()

add_test(cont_file_tests cont_file)