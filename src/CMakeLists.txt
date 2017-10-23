cmake_minimum_required(VERSION 3.5)
project(src)


set(SOURCE_FILES
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

add_library(fs_blk_cache src/block_cache.cpp include/fs270/block_cache.hpp src/block.cpp include/fs270/block.hpp)
target_link_libraries(fs_blk_cache PUBLIC fs_interface)

add_library(fs_inode src/inode.cpp include/fs270/inode.hpp)
target_link_libraries(fs_inode PUBLIC fs_interface)