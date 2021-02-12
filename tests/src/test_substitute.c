#include "testdrive.h"

#include <mintpl/generators.h>
#include <mintpl/substitute.h>

static const mtpl_allocators allocs = { malloc, realloc, free };

FIXTURE(substitution, "Substitution")
    char text[256] = { 0 };
    mtpl_buffer buffer = { .data = text, .cursor = 0, .size = 256 };

    mtpl_hashtable* gens;
    mtpl_htable_create(&allocs, &gens);
    mtpl_generator copy = mtpl_generator_copy;
    mtpl_generator replace = mtpl_generator_replace;
    mtpl_htable_insert(":", &copy, sizeof(mtpl_generator), &allocs, gens);
    mtpl_htable_insert("=", &replace, sizeof(mtpl_generator), &allocs, gens);

    mtpl_result res;
   
    SECTION("Plain text")
        res = mtpl_substitute("foo bar", &allocs, NULL, NULL, &buffer);
        REQUIRE(res == MTPL_SUCCESS);
        REQUIRE(strcmp("foo bar", text) == 0);
    END_SECTION

    SECTION("Quoted")
        res = mtpl_substitute( "{[:>test]}", &allocs, NULL, NULL, &buffer);
        REQUIRE(res == MTPL_SUCCESS);
        REQUIRE(strcmp("[:>test]", text) == 0);
    END_SECTION

    SECTION("Nested")
        res = mtpl_substitute( "[:>foo[:>bar]]", &allocs, gens, NULL, &buffer);
        REQUIRE(res == MTPL_SUCCESS);
        REQUIRE(strcmp("foobar", text) == 0);
    END_SECTION

    SECTION("Replacement")
        mtpl_hashtable* properties;
        mtpl_htable_create(&allocs, &properties);
        mtpl_htable_insert("test1", "foo", 4, &allocs, properties);
        mtpl_htable_insert("test2", "bar", 4, &allocs, properties);
        res = mtpl_substitute(
            "[:>[=>test1][=>test2]]",
            &allocs,
            gens,
            properties,
            &buffer
        );
        REQUIRE(res ==  MTPL_SUCCESS);
        REQUIRE(strcmp("foobar", text) == 0);
    END_SECTION

    SECTION("Escaped")
        res = mtpl_substitute("\\[:>foobar\\]", &allocs, NULL, NULL, &buffer);
        REQUIRE(res == MTPL_SUCCESS);
        REQUIRE(strcmp("[:>foobar]", text) == 0);
    END_SECTION
END_FIXTURE

int main(void) {
    return RUN_TEST(substitution);
}

