cmake_minimum_required(VERSION 2.8)
project(head2mouse)
find_package(OpenCV REQUIRED)
find_package(X11 REQUIRED)
include_directories($ENV{ARUCO}/src)
add_executable(head2mouse head2mouse.cc)
target_link_libraries(head2mouse $ENV{ARUCO}/src/libaruco.so ${OpenCV_LIBS} ${X11_LIBRARIES})
