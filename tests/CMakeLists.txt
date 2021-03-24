cmake_minimum_required(VERSION 3.5)
project(mintpl-tests)

set(TESTS
    test_hashtable
    test_generators
    test_generator_arithmetics
    test_substitute
    test_unicode
)

foreach(T ${TESTS})
    add_executable(${T} src/${T}.c)
    target_link_libraries(${T} mintpl m)
    add_test(${T} ${T})
endforeach()

