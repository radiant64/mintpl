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

void test_generator_has_prop(void** state) {
    char out[32] = { 0 };
    mtpl_buffer buf = { .data = out, .size = 32 };
    mtpl_buffer input1 = { "foo" };

    mtpl_hashtable* properties;
    mtpl_htable_create(&allocs, &properties);
    assert_non_null(properties);

    mtpl_htable_insert("foo", "bar", 4, &allocs, properties);
    
    mtpl_result result = mtpl_generator_has_prop(
        &allocs,
        &input1,
        NULL,
        properties,
        &buf
    );
    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("#t", out);
    
    mtpl_buffer input2 = { "bar" };
    buf.cursor = 0;
    result = mtpl_generator_has_prop(
        &allocs,
        &input2,
        NULL,
        properties,
        &buf
    );
    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("#f", out);
}

void test_generator_let(void** state) {
    mtpl_hashtable* properties;
    mtpl_htable_create(&allocs, &properties);
    
    mtpl_buffer in = { "my\\ test foo bar" };
    mtpl_result res = mtpl_generator_let(&allocs, &in, NULL, properties, NULL);
    assert_int_equal(res, MTPL_SUCCESS);

    const char* found = mtpl_htable_search("my test", properties);
    assert_non_null(found);
    assert_string_equal(found, "foo bar");
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
    mtpl_htable_insert("test", "ok", 3, &allocs, properties);

    mtpl_buffer input1 = { "1;2;3;4 meta [:>[=>meta]! [=>test] ]" };
    mtpl_result result = mtpl_generator_for(
        &allocs,
        &input1,
        generators,
        properties,
        &buf
    );

    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("1! ok 2! ok 3! ok 4! ok ", out);
    
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
    
    memset(out, 0, 32);
    buf = (mtpl_buffer) { .data = out, .size = 32 };
    
    mtpl_buffer input3 = { "#f [=>foo] [:>it's ok]" }; // Trigger 'else'.
    result = mtpl_generator_if(
        &allocs,
        &input3,
        generators,
        properties,
        &buf
    );

    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("it's ok", out);
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

void test_generator_compare(void** state) {
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
    
    buf.cursor = 0;
    mtpl_buffer input3 = { "12 23" };
    result = mtpl_generator_greater(&allocs, &input3, generators, NULL, &buf);
    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("#f", out);
    
    buf.cursor = 0;
    mtpl_buffer input4 = { "23 12" };
    result = mtpl_generator_greater(&allocs, &input4, generators, NULL, &buf);
    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("#t", out);
    
    buf.cursor = 0;
    input3.cursor = 0;
    result = mtpl_generator_less(&allocs, &input3, generators, NULL, &buf);
    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("#t", out);
    
    buf.cursor = 0;
    input4.cursor = 0;
    result = mtpl_generator_less(&allocs, &input4, generators, NULL, &buf);
    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("#f", out);
    
    buf.cursor = 0;
    mtpl_buffer input5 = { "22 22" };
    result = mtpl_generator_gteq(&allocs, &input5, generators, NULL, &buf);
    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("#t", out);
    
    buf.cursor = 0;
    input4.cursor = 0;
    result = mtpl_generator_gteq(&allocs, &input4, generators, NULL, &buf);
    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("#t", out);
    
    buf.cursor = 0;
    input3.cursor = 0;
    result = mtpl_generator_gteq(&allocs, &input3, generators, NULL, &buf);
    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("#f", out);
    
    buf.cursor = 0;
    input5.cursor = 0;
    result = mtpl_generator_lteq(&allocs, &input5, generators, NULL, &buf);
    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("#t", out);
    
    buf.cursor = 0;
    input4.cursor = 0;
    result = mtpl_generator_lteq(&allocs, &input4, generators, NULL, &buf);
    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("#f", out);
    
    buf.cursor = 0;
    input3.cursor = 0;
    result = mtpl_generator_lteq(&allocs, &input3, generators, NULL, &buf);
    assert_int_equal(result, MTPL_SUCCESS);
    assert_string_equal("#t", out);
}

const struct CMUnitTest generators_tests[] = {
    cmocka_unit_test(test_generator_copy),
    cmocka_unit_test(test_generator_replace),
    cmocka_unit_test(test_generator_has_prop),
    cmocka_unit_test(test_generator_let),
    cmocka_unit_test(test_generator_for),
    cmocka_unit_test(test_generator_if),
    cmocka_unit_test(test_generator_not),
    cmocka_unit_test(test_generator_compare)
};

int main(void) {
    return cmocka_run_group_tests(generators_tests, NULL, NULL);
}

