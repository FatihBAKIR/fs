cmake_minimum_required(VERSION 3.8)
project(src)

set(CMAKE_CXX_STANDARD 17)

set(SOURCE_FILES src/main.cpp
        include/fs270/bitmap_allocator.hpp
        include/fs270/config.hpp
        include/fs270/superblock.hpp
        include/fs270/fs_fwd.hpp
        include/fs270/fsexcept.hpp)


add_library(fs_interface INTERFACE)
target_include_directories(fs_interface INTERFACE "include")

add_library(fs_ram_blk src/ram_block_dev.cpp include/fs270/ram_block_dev.hpp)
target_link_libraries(fs_ram_blk PUBLIC fs_interface)

add_library(fs_mmap_blk src/mmap_block_dev.cpp include/fs270/mmap_block_dev.hpp)
target_link_libraries(fs_mmap_blk PUBLIC fs_interface)

add_library(fs_cont_data src/contiguous_data.cpp include/fs270/contiguous_data.hpp)
target_link_libraries(fs_cont_data PUBLIC fs_interface)

#add_executable(src ${SOURCE_FILES})
#target_link_libraries(src PUBLIC fs_mmap_blk fs_cont_data)

