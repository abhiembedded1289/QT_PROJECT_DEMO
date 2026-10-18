cmake_minimum_required(VERSION 3.16)
project(qt_device_hmi LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

add_executable(smart_hmi hmi_main.cpp)
target_link_libraries(smart_hmi PRIVATE Qt6::Widgets Qt6::Network)

add_executable(device_server device_server.c)
if(WIN32)
    target_link_libraries(device_server PRIVATE ws2_32)
endif()