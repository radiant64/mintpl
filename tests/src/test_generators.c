#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <setjmp.h>

#include <cmocka.h>

#include <mintpl/generators.h>

static const mtpl_allocators allocs = { malloc, realloc, free };

void test_generator_copy(void** state) {
    char out[32] = { 0 };
    mtpl_buffer buf = { .data = out, .size = 32 };
    mtpl_buffer input = { "foo" };

    mtpl_result result = mtpl_generator_copy(&allocs, &input, NULL, NULL, &buf);

    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("foo", out);
}

void test_generator_replace(void** state) {
    char out[32] = { 0 };
    mtpl_buffer buf = { .data = out, .size = 32 };
    mtpl_buffer input = { "foo" };

    mtpl_hashtable* properties;
    mtpl_htable_create(&allocs, &properties);
    assert_non_null(properties);

    mtpl_htable_insert("foo", "bar", 4, &allocs, properties);
    mtpl_result result = mtpl_generator_replace(
        &allocs,
        &input,
        NULL,
        properties,
        &buf
    );

    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("bar", out);
}

void test_generator_for(void** state) {
    char out[32] = { 0 };
    mtpl_buffer buf = { .data = out, .size = 32 };

    mtpl_hashtable* generators;
    mtpl_hashtable* properties;
    mtpl_htable_create(&allocs, &generators);
    mtpl_htable_create(&allocs, &properties);
    mtpl_generator copy = mtpl_generator_copy;
    mtpl_generator replace = mtpl_generator_replace;
    mtpl_htable_insert(":", &copy, sizeof(mtpl_generator), &allocs, generators);
    mtpl_htable_insert(
        "=",
        &replace,
        sizeof(mtpl_generator),
        &allocs,
        generators
    );

    mtpl_buffer input1 = { "1;2;3;4 meta [:>[=>meta]! ]" };
    mtpl_result result = mtpl_generator_for(
        &allocs,
        &input1,
        generators,
        properties,
        &buf
    );

    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("1! 2! 3! 4! ", out);
    
    memset(out, 0, 32);
    buf = (mtpl_buffer) { .data = out, .size = 32 };
    mtpl_htable_insert("foo", "baz ", 5, &allocs, properties);
    mtpl_htable_insert("bar", "qux", 4, &allocs, properties);
    
    mtpl_buffer input2 = { "foo;bar meta [=>[=>meta]]" };
    result = mtpl_generator_for(
        &allocs,
        &input2,
        generators,
        properties,
        &buf
    );

    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("baz qux", out);
}

void test_generator_if(void** state) {
    char out[32] = { 0 };
    mtpl_buffer buf = { .data = out, .size = 32 };

    mtpl_hashtable* generators;
    mtpl_hashtable* properties;
    mtpl_htable_create(&allocs, &generators);
    mtpl_htable_create(&allocs, &properties);
    mtpl_generator copy = mtpl_generator_copy;
    mtpl_htable_insert(":", &copy, sizeof(mtpl_generator), &allocs, generators);

    mtpl_buffer input1 = { "#f [=>foo]" }; // Would cause error if evaluated.
    mtpl_result result = mtpl_generator_if(
        &allocs,
        &input1,
        generators,
        properties,
        &buf
    );

    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("", out);
    
    memset(out, 0, 32);
    buf = (mtpl_buffer) { .data = out, .size = 32 };
    
    mtpl_buffer input2 = { "#t [=>foo]" }; // Expect an error.
    result = mtpl_generator_if(
        &allocs,
        &input2,
        generators,
        properties,
        &buf
    );

    assert_int_equal(result, MTPL_ERR_UNKNOWN_KEY);
    assert_string_equal("", out);
}

void test_generator_not(void** state) {
    char out[32] = { 0 };
    mtpl_buffer buf = { .data = out, .size = 32 };

    mtpl_buffer i1 = { "#f" };
    mtpl_result result = mtpl_generator_not(&allocs, &i1, NULL, NULL, &buf);

    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("#t", out);
    
    buf = (mtpl_buffer) { .data = out, .size = 32 };
    
    mtpl_buffer i2 = { "#t" };
    result = mtpl_generator_not(&allocs, &i2, NULL, NULL, &buf);

    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("#f", out);
}

void test_generator_equals(void** state) {
    char out[32] = { 0 };
    mtpl_buffer buf = { .data = out, .size = 32 };

    mtpl_hashtable* generators;
    mtpl_htable_create(&allocs, &generators);
    mtpl_generator copy = mtpl_generator_copy;
    mtpl_htable_insert(":", &copy, sizeof(mtpl_generator), &allocs, generators);

    mtpl_buffer input1 = { "foo [:>foo]" };
    mtpl_result result = mtpl_generator_equals(
        &allocs,
        &input1,
        generators,
        NULL,
        &buf
    );

    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("#t", out);
    
    memset(out, 0, 32);
    buf = (mtpl_buffer) { .data = out, .size = 32 };
    
    mtpl_buffer input2 = { "foo bar" };
    result = mtpl_generator_equals(&allocs, &input2, generators, NULL, &buf);

    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("#f", out);
}

const struct CMUnitTest generators_tests[] = {
    cmocka_unit_test(test_generator_copy),
    cmocka_unit_test(test_generator_replace),
    cmocka_unit_test(test_generator_for),
    cmocka_unit_test(test_generator_if),
    cmocka_unit_test(test_generator_not),
    cmocka_unit_test(test_generator_equals)
};

int main(void) {
    return cmocka_run_group_tests(generators_tests, NULL, NULL);
}

