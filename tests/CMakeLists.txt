cmake_minimum_required(VERSION 3.11)
project(mintpl-tests)

set(TESTS
    test_hashtable
    test_generators
    test_substitute
    test_unicode
)

foreach(T ${TESTS})
    add_executable(${T} src/${T}.c)
    target_link_libraries(${T} mintpl -g)
    add_test(${T} ${T})
endforeach()

