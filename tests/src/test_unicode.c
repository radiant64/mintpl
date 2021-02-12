#include "testdrive.h"

#include <mintpl/generators.h>
#include <mintpl/substitute.h>

static const mtpl_allocators allocs = { malloc, realloc, free };

FIXTURE(unicode, "Unicode preservation")
    mtpl_buffer utf8 = { "\x61\xe4\xb8\xad\xd0\xaf" };
    mtpl_buffer foo = { "foo" };

    char text[256] = { 0 };
    mtpl_buffer buffer = { .data = text, .cursor = 0, .size = 256 };

    mtpl_hashtable* gens;
    mtpl_htable_create(&allocs, &gens);
    REQUIRE(gens);
    mtpl_generator copy = mtpl_generator_copy;
    mtpl_generator replace = mtpl_generator_replace;
    mtpl_htable_insert(":", &copy, sizeof(mtpl_generator), &allocs, gens);
    mtpl_htable_insert("=", &replace, sizeof(mtpl_generator), &allocs, gens);
        
    mtpl_hashtable* props;
    mtpl_htable_create(&allocs, &props);
    REQUIRE(props);
    mtpl_htable_insert(utf8.data, foo.data, strlen(foo.data), &allocs, props);
    mtpl_htable_insert(foo.data, utf8.data, strlen(utf8.data), &allocs, props);

    mtpl_result res;

    SECTION("Plain unicode text is preserved when copying")
        res = mtpl_generator_copy(&allocs, &utf8, NULL, NULL, &buffer);
        REQUIRE(res == MTPL_SUCCESS);
        REQUIRE(strcmp(utf8.data, text) == 0);
    END_SECTION

    SECTION("Unicode can be stored in properties")
        res = mtpl_generator_replace(&allocs, &foo, NULL, props, &buffer);

        REQUIRE(res == MTPL_SUCCESS);
        REQUIRE(strcmp(utf8.data, buffer.data) == 0);
    END_SECTION

    SECTION("Unicode can be used in property names")
        res = mtpl_generator_replace(&allocs, &utf8, NULL, props, &buffer);

        REQUIRE(res == MTPL_SUCCESS);
        REQUIRE(strcmp(foo.data, buffer.data) == 0);
    END_SECTION
END_FIXTURE

int main(void) {
    return RUN_TEST(unicode);
}

