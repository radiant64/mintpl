#include <stdarg.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <setjmp.h>
#include <cmocka.h>

#include <mintpl/descriptor.h>

static mtpl_result test_generator(
    const char* arg,
    const mtpl_allocators* allocators,
    struct mtpl_descriptors* descriptors,
    mtpl_buffer* out_buffer
) { }

static mtpl_result test_generator_alt(
    const char* arg,
    const mtpl_allocators* allocators,
    struct mtpl_descriptors* descriptors,
    mtpl_buffer* out_buffer
) { }

static void insert_dummy_descriptors(
    int count,
    const mtpl_allocators* allocators,
    mtpl_descriptors* descriptors
) {
    char name[16];
    for (int i = 0; i < count; ++i) {
        snprintf(name, 16, "test%02i", i);
        mtpl_insert_descriptor(name, test_generator, allocators, descriptors);
    }
}

void test_create_descriptors(void** state) {
    static const mtpl_allocators allocators = { malloc, realloc, free };
    mtpl_descriptors* descriptors = NULL;

    mtpl_result result = mtpl_create_descriptors(&allocators, &descriptors);

    assert_int_equal(result, MTPL_SUCCESS);
    assert_non_null(descriptors);
    assert_non_null(descriptors->names);
    assert_non_null(descriptors->generators);
    assert_int_equal(descriptors->count, 0);
    assert_int_equal(descriptors->capacity, MTPL_INITIAL_DESCRIPTORS);
}

void test_insert_descriptor(void** state) {
    static const mtpl_allocators allocators = { malloc, realloc, free };
    mtpl_descriptors* descriptors = NULL;
    mtpl_create_descriptors(&allocators, &descriptors);

    mtpl_result result = mtpl_insert_descriptor(
        "test",
        test_generator,
        &allocators,
        descriptors
    );

    assert_int_equal(result, MTPL_SUCCESS);
    assert_non_null(descriptors->names[0]);
    assert_string_equal(descriptors->names[0], "test");
    assert_int_equal(descriptors->count, 1);
    assert_ptr_equal(descriptors->generators[0], test_generator);
}

void test_insert_many_descriptors(void** state) {
    static const mtpl_allocators allocators = { malloc, realloc, free };
    mtpl_descriptors* descriptors = NULL;
    mtpl_create_descriptors(&allocators, &descriptors);

    insert_dummy_descriptors(
        MTPL_INITIAL_DESCRIPTORS + 2,
        &allocators,
        descriptors
    );

    assert_int_equal(descriptors->count, MTPL_INITIAL_DESCRIPTORS + 2);
    assert_not_in_range(descriptors->capacity, 0, MTPL_INITIAL_DESCRIPTORS + 2);
    assert_in_set(
        (uintptr_t) test_generator,
        (uintptr_t*) descriptors->generators,
        MTPL_INITIAL_DESCRIPTORS + 2
    );
    char name[16];
    for (int i = 0; i < MTPL_INITIAL_DESCRIPTORS + 2; ++i) {
        snprintf(name, 16, "test%02i", i);
        assert_string_equal(descriptors->names[i], name);
    }
}

void test_lookup(void** state) {
    static const mtpl_allocators allocators = { malloc, realloc, free };
    mtpl_descriptors* descriptors = NULL;
    mtpl_create_descriptors(&allocators, &descriptors);

    insert_dummy_descriptors(
        MTPL_INITIAL_DESCRIPTORS * 2,
        &allocators,
        descriptors
    );
    descriptors->generators[8] = test_generator_alt;

    mtpl_generator found;
    mtpl_result result = mtpl_lookup("test08", descriptors, &found);

    assert_int_equal(result, MTPL_SUCCESS);
    assert_non_null(found);
    assert_ptr_equal(found, test_generator_alt);
}

const struct CMUnitTest lookup_tests[] = {
    cmocka_unit_test(test_create_descriptors),
    cmocka_unit_test(test_insert_descriptor),
    cmocka_unit_test(test_insert_many_descriptors),
    cmocka_unit_test(test_lookup),
};

int main(void) {
    return cmocka_run_group_tests(lookup_tests, NULL, NULL);
}

