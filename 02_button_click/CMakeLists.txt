cmake_minimum_required(VERSION 3.16)
project(button_click LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(button_click main.cpp)
target_link_libraries(button_click PRIVATE Qt6::Widgets)
