cmake_minimum_required(VERSION 3.8)
project(JoyGameplay)

set(CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-D DEBUG=1)
endif()

add_library(
        JoyGameplay

        Components/RoomBehaviour.cpp)
############## make #include "<folder>/<header>.h" possible ##############
target_include_directories(JoyGameplay PUBLIC .)
target_include_directories(JoyGameplay PUBLIC Libs/glm) # somehow this is not a library
target_include_directories(JoyGameplay PUBLIC Libs/rapidjson/include)





