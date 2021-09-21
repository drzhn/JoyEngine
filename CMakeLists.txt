cmake_minimum_required(VERSION 3.17)
project(JoyMain)

set(CMAKE_CXX_STANDARD 17)

add_executable(JoyMain WIN32 main.cpp WindowHandler.cpp)

add_subdirectory(JoyEngine)
target_link_libraries(JoyMain JoyEngine)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-D DEBUG=1)
endif()
add_compile_options(/P output)