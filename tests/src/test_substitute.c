#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <setjmp.h>

#include <cmocka.h>

#include <mintpl/generators.h>
#include <mintpl/substitute.h>

static const mtpl_allocators allocators = { malloc, realloc, free };

void test_plaintext(void** state) {
    char text[256] = { 0 };
    mtpl_buffer buffer = { .data = text, .cursor = 0, .size = 256 };
    mtpl_result result = mtpl_substitute("foo bar", NULL, NULL, &buffer);
    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("foo bar", text);
}

void test_nested(void** state) {
    char text[256] = { 0 };
    mtpl_buffer buffer = { .data = text, .cursor = 0, .size = 256 };
    mtpl_hashtable* generators;
    mtpl_htable_create(&allocators, &generators);
    mtpl_generator copy = mtpl_generator_copy;
    mtpl_htable_insert(
        ":",
        &copy,
        sizeof(mtpl_generator),
        &allocators,
        generators
    );
    mtpl_result result = mtpl_substitute(
        "[:>foo [:>bar]]",
        generators,
        NULL,
        &buffer
    );
    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("foo bar", text);
}

void test_replace(void** state) {
    char text[256] = { 0 };
    mtpl_buffer buffer = { .data = text, .cursor = 0, .size = 256 };
    mtpl_hashtable* generators;
    mtpl_hashtable* properties;
    mtpl_htable_create(&allocators, &generators);
    mtpl_htable_create(&allocators, &properties);
    mtpl_generator copy = mtpl_generator_copy;
    mtpl_generator replace = mtpl_generator_replace;
    mtpl_htable_insert(
        ":",
        &copy,
        sizeof(mtpl_generator),
        &allocators,
        generators
    );
    mtpl_htable_insert(
        "=",
        &replace,
        sizeof(mtpl_generator),
        &allocators,
        generators
    );
    mtpl_htable_insert(
        "test1",
        "foo",
        4,
        &allocators,
        properties
    );
    mtpl_htable_insert(
        "test2",
        "bar",
        4,
        &allocators,
        properties
    );
    mtpl_result result = mtpl_substitute(
        "[:>[=>test1] [=>test2]]",
        generators,
        properties,
        &buffer
    );
    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("foo bar", text);
}

void test_escape(void** state) {
    char text[256] = { 0 };
    mtpl_buffer buffer = { .data = text, .cursor = 0, .size = 256 };
    mtpl_result result = mtpl_substitute("\\[:>foo bar\\]", NULL, NULL, &buffer);
    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("[:>foo bar]", text);
}

const struct CMUnitTest substitute_tests[] = {
    cmocka_unit_test(test_plaintext),
    cmocka_unit_test(test_nested),
    cmocka_unit_test(test_replace),
    cmocka_unit_test(test_escape)
};

int main(void) {
    return cmocka_run_group_tests(substitute_tests, NULL, NULL);
}

