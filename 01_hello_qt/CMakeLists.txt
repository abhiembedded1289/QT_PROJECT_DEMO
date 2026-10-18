cmake_minimum_required(VERSION 3.16)
project(hello_qt LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(hello_qt main.cpp)
target_link_libraries(hello_qt PRIVATE Qt6::Widgets)
