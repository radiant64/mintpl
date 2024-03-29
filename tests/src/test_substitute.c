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

    SECTION("Initial whitespace")
        res = mtpl_substitute("[:> foo]", &allocs, gens, NULL, &buffer);
        REQUIRE(res == MTPL_SUCCESS);
        REQUIRE(strcmp("foo", text) == 0);
    END_SECTION

    SECTION("Unknown generator")
        res = mtpl_substitute("[foo>test]", &allocs, gens, NULL, &buffer);
        REQUIRE(res == MTPL_ERR_UNKNOWN_KEY);
    END_SECTION

    SECTION("Syntax error")
        res = mtpl_substitute("]", &allocs, gens, NULL, &buffer);
        REQUIRE(res == MTPL_ERR_SYNTAX);

        res = mtpl_substitute("{", &allocs, gens, NULL, &buffer);
        REQUIRE(res == MTPL_ERR_SYNTAX);

        res = mtpl_substitute("[", &allocs, gens, NULL, &buffer);
        REQUIRE(res == MTPL_ERR_SYNTAX);

        res = mtpl_substitute("[:>test]]", &allocs, gens, NULL, &buffer);
        REQUIRE(res == MTPL_ERR_SYNTAX);

        res = mtpl_substitute("{test", &allocs, gens, NULL, &buffer);
        REQUIRE(res == MTPL_ERR_SYNTAX);

        res = mtpl_substitute("[:>test", &allocs, gens, NULL, &buffer);
        REQUIRE(res == MTPL_ERR_SYNTAX);

        res = mtpl_substitute("[foo]", &allocs, gens, NULL, &buffer);
        REQUIRE(res == MTPL_ERR_SYNTAX);
    END_SECTION

    SECTION("Quoted")
        res = mtpl_substitute("{[:>test]}", &allocs, NULL, NULL, &buffer);
        REQUIRE(res == MTPL_SUCCESS);
        REQUIRE(strcmp("[:>test]", text) == 0);
        SECTION("Empty")
            res = mtpl_substitute("{}", &allocs, NULL, NULL, &buffer);
            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(text[buffer.cursor] == 0);
        END_SECTION
    END_SECTION

    SECTION("Nested")
        res = mtpl_substitute("[:>foo[:>bar]]", &allocs, gens, NULL, &buffer);
        REQUIRE(res == MTPL_SUCCESS);
        REQUIRE(strcmp("foobar", text) == 0);
    END_SECTION

    SECTION("Empty")
        res = mtpl_substitute("[:>]", &allocs, gens, NULL, &buffer);
        REQUIRE(res == MTPL_SUCCESS);
        REQUIRE(text[0] == 0);
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

