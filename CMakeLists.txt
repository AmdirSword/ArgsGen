cmake_minimum_required(VERSION 3.14)
project(ArgsGen)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14 -Ofast -ffast-math -march=native -DDEBUG")

add_executable(ArgsGen main.cpp)
