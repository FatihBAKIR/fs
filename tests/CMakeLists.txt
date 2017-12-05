cmake_minimum_required(VERSION 3.5)
project(fs CXX)

if (FS_BUILD_TESTS)
    message(STATUS "Building tests...")
add_executable(fs_test
        cont_file_tests.cpp
        tests_main.cpp
        cache_tests.cpp
        inode_tests.cpp
        mmap_tests.cpp
        mkfs_tests.cpp
        bitmap_tests.cpp
        tests_common.cpp
        ilist_tests.cpp
        block_device_tests.cpp
        dir_tests.cpp)
target_link_libraries(fs_test PRIVATE
        fs_cont_data fs_ram_blk fs_blk_cache fs_inode
        fs_mmap_blk fs_mkfs fs_bm_alloc fs_instance
        fs_dir fs_disk_blk)
enable_testing()
add_test(fs_tests fs_test)
endif()