cmake_minimum_required(VERSION 3.11)
project(mintpl 
    VERSION 1.1.0
    DESCRIPTION "Minimal Template Programming Language"
)
include_directories(include)

set(SOURCES
    src/buffers.c
    src/hashtable.c
    src/generators.c
    src/mintpl.c
    src/substitute.c
)

if(SHARED_LIBRARY)
    add_library(${PROJECT_NAME} SHARED ${SOURCES})
else()
    add_library(${PROJECT_NAME} STATIC ${SOURCES})
endif()
set_target_properties(${PROJECT_NAME} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
)
configure_file(mintpl.pc.in mintpl.pc @ONLY)
include(GNUInstallDirs)
install(
    TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(
    FILES ${CMAKE_BINARY_DIR}/mintpl.pc
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig
)

add_subdirectory(standalone)
add_subdirectory(tests)

