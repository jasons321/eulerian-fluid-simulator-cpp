cmake_minimum_required(VERSION 3.16)
project(hello_raylib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(raylib REQUIRED)

add_executable(hello main.cpp fluid.cpp)
target_link_libraries(hello PRIVATE raylib)

if(APPLE)
    target_link_libraries(hello PRIVATE
        "-framework IOKit"
        "-framework Cocoa"
        "-framework OpenGL"
    )
endif()
