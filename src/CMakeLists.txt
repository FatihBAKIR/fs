cmake_minimum_required(VERSION 3.8)
project(src)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES src/main.cpp
        src/in_memory_data.cpp
        include/fs270/in_memory_data.hpp
        include/fs270/bitmap_allocator.hpp
        include/fs270/config.hpp
        include/fs270/mapped_file_provider.hpp src/mapped_file_provider.cpp)
add_executable(src ${SOURCE_FILES})

target_include_directories(src PUBLIC "include")