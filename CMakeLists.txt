cmake_minimum_required(VERSION 3.5)
project(mintpl 
    VERSION 3.3.2
    DESCRIPTION "Minimal Template Programming Language"
)

set(SOURCES
    src/buffers.c
    src/hashtable.c
    src/generators.c
    src/generator_arithmetics.c
    src/mintpl.c
    src/substitute.c
    src/version.c
)

if(SHARED_LIBRARY)
    add_library(${PROJECT_NAME} SHARED ${SOURCES})
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCES})
endif()
target_include_directories(${PROJECT_NAME} PUBLIC include)
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)
configure_file(mintpl.pc.in mintpl.pc @ONLY@)
include(GNUInstallDirs)
install(
    TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    FILES_MATCHING PATTERN "*.h"
)
install(
    FILES ${CMAKE_BINARY_DIR}/mintpl.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
)

add_subdirectory(standalone)

if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

