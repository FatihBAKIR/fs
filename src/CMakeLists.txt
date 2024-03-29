cmake_minimum_required(VERSION 3.5)
project(src)

set(SOURCE_FILES
        include/fs270/config.hpp
        include/fs270/superblock.hpp
        include/fs270/fs_fwd.hpp
        include/fs270/fsexcept.hpp)

add_library(fs_interface INTERFACE)
target_include_directories(fs_interface INTERFACE "include")
#target_compile_features(fs_interface INTERFACE cxx_std_17)

add_library(fs_ram_blk src/ram_block_dev.cpp include/fs270/ram_block_dev.hpp)
target_link_libraries(fs_ram_blk PUBLIC fs_interface)

add_library(fs_mmap_blk src/mmap_block_dev.cpp include/fs270/mmap_block_dev.hpp)
target_link_libraries(fs_mmap_blk PUBLIC fs_interface)

add_library(fs_disk_blk src/disk_block_device.cpp include/fs270/disk_block_dev.hpp)
target_link_libraries(fs_disk_blk PUBLIC fs_interface)

add_library(fs_cont_data src/contiguous_data.cpp include/fs270/contiguous_data.hpp)
target_link_libraries(fs_cont_data PUBLIC fs_interface fs_blk_cache)

find_package(Boost REQUIRED COMPONENTS system)

add_library(fs_blk_cache
        src/block_cache.cpp include/fs270/block_cache.hpp
        src/block.cpp include/fs270/block.hpp)
target_link_libraries(fs_blk_cache PUBLIC fs_interface)
target_link_libraries(fs_blk_cache PUBLIC ${Boost_LIBRARIES} ${TBB_LIBRARY} pthread)
target_include_directories(fs_blk_cache PUBLIC ${Boost_INCLUDE_DIR})

add_library(fs_inode src/inode.cpp include/fs270/inode.hpp)
target_link_libraries(fs_inode PUBLIC fs_interface fs_cont_data fs_blk_cache fs_instance)

add_library(fs_mkfs src/mkfs.cpp include/fs270/mkfs.hpp)
target_link_libraries(fs_mkfs PUBLIC fs_interface fs_blk_cache fs_bm_alloc fs_mmap_blk fs_disk_blk fs_instance)

add_library(fs_instance src/fs_instance.cpp include/fs270/fs_instance.hpp)
target_link_libraries(fs_instance PUBLIC fs_interface fs_inode)

add_library(fs_bm_alloc src/bitmap_allocator.cpp include/fs270/bitmap_allocator.hpp)
target_link_libraries(fs_bm_alloc PUBLIC fs_interface fs_blk_cache)

add_library(fs_dir include/fs270/directory.hpp src/directory.cpp)
target_link_libraries(fs_dir PUBLIC fs_inode ${Boost_LIBRARIES})
target_include_directories(fs_dir PUBLIC ${Boost_INCLUDE_DIR})

add_library(fs_symlink include/fs270/symlink.hpp src/symlink.cpp)
target_link_libraries(fs_symlink PUBLIC fs_inode)

find_package(FUSE)
find_package(OSXFuse)
if (UNIX AND (FUSE_FOUND OR OSXFUSE_FOUND))
    message(STATUS "Fuse found, building fuse_driver")
    add_executable(fuse_driver src/fuse_driver.cpp)
    target_link_libraries(fuse_driver PRIVATE
            fs_cont_data fs_blk_cache fs_inode
            fs_mmap_blk fs_mkfs fs_instance
            fs_dir)
    target_compile_definitions(fuse_driver PRIVATE _FILE_OFFSET_BITS=64 FUSE_USE_VERSION=31)

    if (UNIX AND NOT APPLE)
        target_include_directories(fuse_driver PRIVATE ${FUSE_INCLUDE_DIRS})
        target_link_libraries(fuse_driver PRIVATE ${FUSE_LIBRARIES} pthread)
        target_link_libraries(fuse_driver PRIVATE -static-libgcc -static-libstdc++)
    elseif(UNIX AND APPLE)
        target_include_directories(fuse_driver PRIVATE ${OSXFUSE_INCLUDE_DIRS})
        target_link_libraries(fuse_driver PUBLIC ${OSXFUSE_LIBRARIES})
    endif()
endif()

add_executable(debug_driver src/debug_driver.cpp)
target_link_libraries(debug_driver PRIVATE fs_cont_data fs_blk_cache fs_inode fs_mmap_blk fs_mkfs fs_instance fs_dir)

add_executable(mkfs src/mkfs_driver.cpp)

target_link_libraries(mkfs PRIVATE fs_cont_data fs_blk_cache fs_inode fs_mkfs fs_dir)
if (UNIX AND NOT APPLE)
    target_link_libraries(mkfs PRIVATE -static-libgcc -static-libstdc++)
endif()
