cmake_minimum_required(VERSION 3.16)
project(smart_meter LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(smart_meter main.cpp)
target_link_libraries(smart_meter PRIVATE Qt6::Widgets)