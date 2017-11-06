cmake_minimum_required(VERSION 3.8)
project(proj2)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 11)

add_definitions(-D_DEBUG)

include_directories(
        ./include
        C:/Users/YK/Source/linux_headers
        C:/Users/YK/Source/linux_headers/gcc/include
)

# TEST: display_task
set(SOURCE_FILES
        include/AddrInfo.h
        include/ClientConnection.h
        include/helpers.h
        include/HttpRequest.h
        include/HttpResponse.h
        include/HttpUri.h
        include/ListenConnection.h
        include/LineScanner.h
        include/HttpHeaderBuilder.h

        src/AddrInfo.cpp
        src/ClientConnection.cpp
        src/helpers.cpp
        src/HttpRequest.cpp
        src/HttpResponse.cpp
        src/HttpUri.cpp
        src/ListenConnection.cpp
        src/proxy.cpp
        src/LineScanner.cpp
        src/HttpHeaderBuilder.cpp
        include/ScopeGuard.h src/ForwardSocketData.cpp src/ForwardSocketData.h)

add_executable(proj2 ${SOURCE_FILES})
