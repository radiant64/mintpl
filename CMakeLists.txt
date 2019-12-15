cmake_minimum_required(VERSION 3.11)
project(mintpl)
include_directories(include)

set(SOURCES
    src/descriptor.c
    src/generators.c
    src/substitute.c
)

add_library(${PROJECT_NAME} ${SOURCES})
add_subdirectory(tests)

