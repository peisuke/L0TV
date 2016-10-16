cmake_minimum_required(VERSION 2.8)
project(L0TV)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake-modules)
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
add_library(L0TV L0TV.cpp)
target_link_libraries(L0TV  ${OpenCV_LIBRARIES})
