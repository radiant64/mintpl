#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <setjmp.h>

#include <cmocka.h>

#include <mintpl/generators.h>

static const mtpl_allocators allocs = { malloc, realloc, free };

void test_generator_copy(void** state) {
    char out[32] = { 0 };
    mtpl_buffer buf = { .data = out, .size = 32 };

    mtpl_result result = mtpl_generator_copy("foo", &allocs, NULL, NULL, &buf);

    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("foo", out);
}

void test_generator_replace(void** state) {
    char out[32] = { 0 };
    mtpl_buffer buf = { .data = out, .size = 32 };

    mtpl_hashtable* properties;
    mtpl_htable_create(&allocs, &properties);
    assert_non_null(properties);

    mtpl_htable_insert("foo", "bar", 4, &allocs, properties);
    mtpl_result result = mtpl_generator_replace(
        "foo",
        &allocs,
        NULL,
        properties,
        &buf
    );

    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("bar", out);
}

const struct CMUnitTest generators_tests[] = {
    cmocka_unit_test(test_generator_copy),
    cmocka_unit_test(test_generator_replace)
};

int main(void) {
    return cmocka_run_group_tests(generators_tests, NULL, NULL);
}

