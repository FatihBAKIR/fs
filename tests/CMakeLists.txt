cmake_minimum_required(VERSION 3.5)
project(fs CXX)

add_executable(fs_test cont_file_tests.cpp tests_main.cpp cache_tests.cpp inode_tests.cpp mmap_tests.cpp mkfs_tests.cpp)
target_link_libraries(fs_test PRIVATE fs_cont_data fs_ram_blk fs_blk_cache fs_inode fs_mmap_blk fs_mkfs)

enable_testing()

add_test(fs_tests fs_test)
