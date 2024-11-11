cmake_minimum_required(VERSION 3.18)
project(bvh LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(3pty)
add_subdirectory(bvh)
