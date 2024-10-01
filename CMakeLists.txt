cmake_minimum_required(VERSION 3.29)
project(SltMachine)

include(FetchContent)
set(BUILD_SHARED_LIBS OFF)
FETCHCONTENT_DECLARE(SFML GIT_REPOSITORY https://github.com/SFML/SFML.git GIT_TAG 2.5.1)
FETCHCONTENT_MAKEAVAILABLE(SFML)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")

set(CMAKE_CXX_STANDARD 14)

add_executable(SltMachine main.cpp
)

target_link_libraries(SltMachine sfml-graphics sfml-audio)