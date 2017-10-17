cmake_minimum_required(VERSION 3.8)
project(src)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES src/main.cpp
        src/ram_block_dev.cpp
        include/fs270/ram_block_dev.hpp
        include/fs270/bitmap_allocator.hpp
        include/fs270/config.hpp
        include/fs270/mmap_block_dev.hpp src/mmap_block_dev.cpp include/fs270/superblock.hpp include/fs270/fs_fwd.hpp include/fs270/contiguous_data.hpp src/contiguous_data.cpp)
add_executable(src ${SOURCE_FILES})

target_include_directories(src PUBLIC "include")