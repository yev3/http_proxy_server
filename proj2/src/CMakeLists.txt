cmake_minimum_required(VERSION 3.8)
project(proj1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)
include_directories(
        .
        C:/Users/YK/Source/linux_headers
        C:/Users/YK/Source/linux_headers/gcc/include
)

# TEST: display_task
set(SOURCE_FILES
        fingerserver.cpp
)

add_executable(proj1 ${SOURCE_FILES})
