cmake_minimum_required(VERSION 3.10)
project(OpenCV_Test)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(OpenCV_Test main.cpp)
target_link_libraries(OpenCV_Test ${OpenCV_LIBS})
