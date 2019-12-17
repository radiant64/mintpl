cmake_minimum_required(VERSION 3.11)
project(mintpl-tests)
include(../cmake/CMocka.cmake)

set(TESTS
    test_hashtable
    test_generators
    test_substitute
)

foreach(T ${TESTS})
    add_executable(${T} src/${T}.c)
    target_link_libraries(${T} mintpl cmocka)
    add_test(${T} ${T})
endforeach()

