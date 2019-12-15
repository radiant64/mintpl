include(FetchContent)

FetchContent_Declare(
    cmocka
    GIT_REPOSITORY git://git.cryptomilk.org/projects/cmocka.git
    GIT_TAG cmocka-1.1.5
)

FetchContent_GetProperties(cmocka)
if(NOT cmocka_POPULATED)
    FetchContent_Populate(cmocka)
endif()

include_directories("${cmocka_SOURCE_DIR}/include")
add_subdirectory(${cmocka_SOURCE_DIR} ${cmocka_BINARY_DIR})

