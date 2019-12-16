#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <setjmp.h>

#include <cmocka.h>

#include <mintpl/hashtable.h>

static const mtpl_allocators allocators = { malloc, realloc, free };

void test_htable_create(void** state) {
    mtpl_hashtable* htable;
    mtpl_result result = mtpl_htable_create(&allocators, &htable);

    assert_int_equal(result, MTPL_SUCCESS);
    assert_non_null(htable);
    assert_non_null(htable->entries);
    assert_int_equal(htable->count, 0);
    assert_int_not_equal(htable->size, 0);

    mtpl_htable_free(&allocators, htable);
}

void test_insert(void** state) {
    char test_string[] = "foo bar";
    mtpl_hashtable* htable;
    mtpl_htable_create(&allocators, &htable);

    mtpl_result result = mtpl_htable_insert(
        "test",
        test_string,
        sizeof(test_string),
        &allocators,
        htable
    );

    assert_int_equal(result, MTPL_SUCCESS);
    assert_int_equal(htable->count, 1);

    mtpl_htable_free(&allocators, htable);
}

void test_search(void** state) {
    char test_string[] = "foo bar";
    mtpl_hashtable* htable;
    mtpl_htable_create(&allocators, &htable);
    mtpl_htable_insert(
        "test",
        test_string,
        sizeof(test_string),
        &allocators,
        htable
    );

    char* found = mtpl_htable_search("test", htable);

    assert_string_equal(found, test_string);
    assert_ptr_not_equal(found, test_string);

    mtpl_htable_free(&allocators, htable);
}

void test_delete(void** state) {
    char test_string[] = "foo bar";
    mtpl_hashtable* htable;
    mtpl_htable_create(&allocators, &htable);
    mtpl_htable_insert(
        "test",
        test_string,
        sizeof(test_string),
        &allocators,
        htable
    );

    mtpl_result result = mtpl_htable_delete("test", &allocators, htable);
    char* found = mtpl_htable_search("test", htable);
    
    assert_int_equal(result, MTPL_SUCCESS);
    assert_null(found);
    
    mtpl_htable_free(&allocators, htable);
}

const struct CMUnitTest hashtable_tests[] = {
    cmocka_unit_test(test_htable_create),
    cmocka_unit_test(test_insert),
    cmocka_unit_test(test_search),
    cmocka_unit_test(test_delete)
};

int main(void) {
    return cmocka_run_group_tests(hashtable_tests, NULL, NULL);
}

