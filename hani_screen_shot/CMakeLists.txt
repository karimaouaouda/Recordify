# cmake_minimum_required(VERSION 3.10)
# project(video-generator)

# find_package(OpenCV REQUIRED)
# # Include OpenCV headers
# include_directories(${OpenCV_INCLUDE_DIRS})

# # include_directories(${OpenCV_INCLUDE_DIRS})

# add_executable(video-generator video-generator.cpp)
# target_link_libraries(video-generator ${OpenCV_LIBS})

# # message helper
# message(STATUS "OpenCV include directories: ${OpenCV_INCLUDE_DIRS}")
# message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")




cmake_minimum_required(VERSION 3.10)
project(video-generator)

find_package(OpenCV REQUIRED)
# Include OpenCV headers
include_directories(${OpenCV_INCLUDE_DIRS})

# include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(video-generator video-generator.cpp)
target_link_libraries(video-generator ${OpenCV_LIBS})

# message helper
message(STATUS "OpenCV include directories: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "OpenCV libraries: ${OpenCV_LIBS}")
