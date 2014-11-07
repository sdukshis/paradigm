cmake_minimum_required(VERSION 2.8)

set(TARGET appcontainer)

project(${TARGET} CXX)

# Compiler options
add_definitions(-Wall)
add_definitions(-std=c++11)

set(SOURCES
    ${CMAKE_SOURCE_DIR}/src/main.cpp
)

add_executable(${TARGET} ${SOURCES})

