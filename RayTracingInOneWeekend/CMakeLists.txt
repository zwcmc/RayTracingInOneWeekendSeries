cmake_minimum_required(VERSION 3.10)

set(PROJECT_NAME RayTracingInOneWeekend)

project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(RayTracingInOneWeekend main.cpp)