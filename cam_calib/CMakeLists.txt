cmake_minimum_required(VERSION 2.8)

project(cam_calib)
find_package(OpenCV 3 REQUIRED)
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})
add_executable(${PROJECT_NAME} "main.cpp")

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
