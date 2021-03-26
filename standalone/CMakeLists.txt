cmake_minimum_required(VERSION 3.5)
project(mintpl-cli)

set(SOURCES
    src/mintpl.c
    src/locale.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} mintpl m)

