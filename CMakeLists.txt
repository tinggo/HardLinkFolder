cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(HardLinkFolder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(HardLinkFolder source/main.cpp source/hardlink.cpp source/worker.h)
