cmake_minimum_required(VERSION 3.8)
project(proj2)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

include_directories(
        ./include
        C:/Users/YK/Source/linux_headers
        C:/Users/YK/Source/linux_headers/gcc/include
)

# TEST: display_task
set(SOURCE_FILES
        src/HttpUri.cpp
        src/helpers.cpp
        src/test.cpp
)

add_executable(proj2 ${SOURCE_FILES})
