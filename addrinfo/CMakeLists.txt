cmake_minimum_required(VERSION 3.15)
project(addrinfo_server)

set(CMAKE_CXX_STANDARD 20)

SET(CMAKE_CXX_FLAGS -pthread)

add_executable(
        addrinfo_server
        main.cpp
        server.cpp
        server.h
        socket_raii.cpp
        socket_raii.h
        epoll_raii.cpp
        epoll_raii.h
        addrinfo_raii.cpp
        addrinfo_raii.h
        timer_raii.cpp
        timer_raii.h)