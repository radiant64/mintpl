#include "testdrive.h"

#include <mintpl/generators.h>

#include <string.h>

static const mtpl_allocators allocs = { malloc, realloc, free };

FIXTURE(generators, "Generators")
    char out[32] = { 0 };
    mtpl_buffer buf = { .data = out, .size = 32 };

    mtpl_hashtable* props;
    mtpl_htable_create(&allocs, &props);
    REQUIRE(props);
        
    mtpl_hashtable* gens;
    mtpl_htable_create(&allocs, &gens);
    REQUIRE(gens);

    mtpl_generator copy = mtpl_generator_copy;
    mtpl_generator replace = mtpl_generator_replace;
    mtpl_htable_insert("=", &replace, sizeof(mtpl_generator), &allocs, gens);
    mtpl_htable_insert(":", &copy, sizeof(mtpl_generator), &allocs, gens);
        
    mtpl_result res;

    SECTION("copy")
        mtpl_buffer input = { "foo" };

        res = mtpl_generator_copy(&allocs, &input, NULL, NULL, &buf);

        REQUIRE(res == MTPL_SUCCESS);
        REQUIRE(strcmp("foo", out) == 0);
    END_SECTION
    SECTION("copy_strip")
        mtpl_buffer input = { " foo\nbar\n\n\n " };

        res = mtpl_generator_copy_strip(&allocs, &input, NULL, NULL, &buf);

        REQUIRE(res == MTPL_SUCCESS);
        REQUIRE(strcmp("foo\nbar", out) == 0);
    END_SECTION

    SECTION("replace")
        mtpl_buffer input = { "foo" };

        mtpl_htable_insert("foo", "bar", 4, &allocs, props);
        res = mtpl_generator_replace(&allocs, &input, NULL, props, &buf);

        REQUIRE(res == MTPL_SUCCESS);
        REQUIRE(strcmp("bar", out) == 0);
    END_SECTION

    SECTION("has_prop")
        SECTION("Property exists")
            mtpl_buffer input = { "foo" };
            mtpl_htable_insert("foo", "bar", 4, &allocs, props);

            res = mtpl_generator_has_prop(&allocs, &input, NULL, props, &buf);
            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(strcmp("#t", out) == 0);
        END_SECTION

        SECTION("Property doesn't exist")
            mtpl_buffer input = { "bar" };

            res = mtpl_generator_has_prop(&allocs, &input, NULL, props, &buf);
            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(strcmp("#f", out) == 0);
        END_SECTION
    END_SECTION

    SECTION("escape")
        mtpl_buffer in = { "foo bar" };

        mtpl_result res = mtpl_generator_escape(&allocs, &in, NULL, NULL, &buf);
        REQUIRE(res == MTPL_SUCCESS);
        REQUIRE(strcmp(out, "foo\\ bar") == 0);
    END_SECTION

    SECTION("let")
        mtpl_buffer in = { "my\\ test foo bar" };
        res = mtpl_generator_let(&allocs, &in, NULL, props, NULL);
        REQUIRE(res == MTPL_SUCCESS);

        const char* found = mtpl_htable_search("my test", props);
        REQUIRE(found);
        REQUIRE(strcmp(found, "foo bar") == 0);

        SECTION("Empty value")
            mtpl_buffer in = { "my\\ test {}" };
            res = mtpl_generator_let(&allocs, &in, NULL, props, NULL);
            REQUIRE(res == MTPL_SUCCESS);

            const char* found = mtpl_htable_search("my test", props);
            REQUIRE(found);
            REQUIRE(found[0] == 0);
        END_SECTION
    END_SECTION

    SECTION("macro")
        mtpl_buffer in = { "operation foo;bar [=>foo][=>bar]" };
        res = mtpl_generator_macro(&allocs, &in, gens, props, NULL);
        REQUIRE(res == MTPL_SUCCESS);

        SECTION("expand")
            mtpl_buffer in = { "operation 123 456" };
            res = mtpl_generator_expand(&allocs, &in, gens, props, &buf);
            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(strcmp(out, "123456") == 0);
        END_SECTION
    END_SECTION

    SECTION("for")
        SECTION("Simple loop")
            mtpl_htable_insert("test", "ok", 3, &allocs, props);

            mtpl_buffer input = { "1;2;3;4 meta [:>[=>meta]! [=>test] ]" };
            res = mtpl_generator_for(&allocs, &input, gens, props, &buf);

            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(strcmp("1! ok 2! ok 3! ok 4! ok ", out) == 0);
        END_SECTION
        
        SECTION("Indirect expansion")
            mtpl_htable_insert("foo", "baz ", 5, &allocs, props);
            mtpl_htable_insert("bar", "qux", 4, &allocs, props);
            
            mtpl_buffer input = { "foo;bar meta [=>[=>meta]]" };
            res = mtpl_generator_for(&allocs, &input, gens, props, &buf);

            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(strcmp("baz qux", out) == 0);
        END_SECTION
    END_SECTION

    SECTION("if")
        SECTION("#f condition is not evaluated")
            // "foo" does not exist; looking it up would cause an error.
            mtpl_buffer input = { "#f [=>foo]" };
            res = mtpl_generator_if(&allocs, &input, gens, props, &buf);

            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(out[0] == '\0');
        END_SECTION
        
        SECTION("#t condition is evaluated")
            mtpl_buffer input = { "#t [=>foo]" }; // Expect an error.
            res = mtpl_generator_if(&allocs, &input, gens, props, &buf);

            REQUIRE(res == MTPL_ERR_UNKNOWN_KEY);
            REQUIRE(out[0] == '\0');
        END_SECTION
        
        SECTION("'else' clause is evaluated on #f condition")
            mtpl_buffer input = { "#f\n[=>foo]\n[:>it's ok]" }; // Trigger 'else'.
            res = mtpl_generator_if(&allocs, &input, gens, props, &buf);

            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(strcmp("it's ok", out) == 0);
        END_SECTION
    END_SECTION

    SECTION("not")
        SECTION("#f -> #t")
            mtpl_buffer input = { "#f" };
            res = mtpl_generator_not(&allocs, &input, NULL, NULL, &buf);

            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(strcmp("#t", out) == 0);
        END_SECTION
        
        SECTION("#t -> #f")
            mtpl_buffer input = { "#t" };
            res = mtpl_generator_not(&allocs, &input, NULL, NULL, &buf);

            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(strcmp("#f", out) == 0);
        END_SECTION
    END_SECTION

    SECTION("Comparison")
        mtpl_buffer i1223 = { "12 23" };
        mtpl_buffer i2222 = { "22 22" };
        mtpl_buffer i2312 = { "23 12" };

        SECTION("equals")
            SECTION("Equality results in #t")
                mtpl_buffer input = { "foo [:>foo]" };
                res = mtpl_generator_equals(&allocs, &input, gens, NULL, &buf);
                REQUIRE(res == MTPL_SUCCESS);
                REQUIRE(strcmp("#t", out) == 0);
            END_SECTION
            SECTION("Non-equality results in #f")
                mtpl_buffer input = { "foo bar" };
                res = mtpl_generator_equals(&allocs, &input, gens, NULL, &buf);

                REQUIRE(res == MTPL_SUCCESS);
                REQUIRE(strcmp("#f", out) == 0);
            END_SECTION
        END_SECTION
        
        SECTION("greater")
            SECTION("Greater results in #t")
                res = mtpl_generator_greater(&allocs, &i2312, gens, NULL, &buf);
                REQUIRE(res == MTPL_SUCCESS);
                REQUIRE(strcmp("#t", out) == 0);
            END_SECTION
            SECTION("Not greater results in #f")
                res = mtpl_generator_greater(&allocs, &i1223, gens, NULL, &buf);
                REQUIRE(res == MTPL_SUCCESS);
                REQUIRE(strcmp("#f", out) == 0);
            END_SECTION
        END_SECTION
        
        SECTION("less")
            SECTION("Less results in #t")
                res = mtpl_generator_less(&allocs, &i1223, gens, NULL, &buf);
                REQUIRE(res == MTPL_SUCCESS);
                REQUIRE(strcmp("#t", out) == 0);
            END_SECTION
            SECTION("Not less results in #f")
                res = mtpl_generator_less(&allocs, &i2312, gens, NULL, &buf);
                REQUIRE(res == MTPL_SUCCESS);
                REQUIRE(strcmp("#f", out) == 0);
            END_SECTION
        END_SECTION
        
        SECTION("gteq")
            SECTION("Equal results in #t")
                res = mtpl_generator_gteq(&allocs, &i2222, gens, NULL, &buf);
                REQUIRE(res == MTPL_SUCCESS);
                REQUIRE(strcmp("#t", out) == 0);
            END_SECTION
            
            SECTION("Greater results in #t")
                res = mtpl_generator_gteq(&allocs, &i2312, gens, NULL, &buf);
                REQUIRE(res == MTPL_SUCCESS);
                REQUIRE(strcmp("#t", out) == 0);
            END_SECTION
            
            SECTION("Less results in #f")
                res = mtpl_generator_gteq(&allocs, &i1223, gens, NULL, &buf);
                REQUIRE(res == MTPL_SUCCESS);
                REQUIRE(strcmp("#f", out) == 0);
            END_SECTION
        END_SECTION

        SECTION("lteq")
            SECTION("Equal results in #t")
                res = mtpl_generator_lteq(&allocs, &i2222, gens, NULL, &buf);
                REQUIRE(res == MTPL_SUCCESS);
                REQUIRE(strcmp("#t", out) == 0);
            END_SECTION
            
            SECTION("Less results in #t")
                res = mtpl_generator_lteq(&allocs, &i1223, gens, NULL, &buf);
                REQUIRE(res == MTPL_SUCCESS);
                REQUIRE(strcmp("#t", out) == 0);
            END_SECTION
            
            SECTION("Greater results in #f")
                res = mtpl_generator_lteq(&allocs, &i2312, gens, NULL, &buf);
                REQUIRE(res == MTPL_SUCCESS);
                REQUIRE(strcmp("#f", out) == 0);
            END_SECTION
        END_SECTION
    END_SECTION

    SECTION("startsw")
        SECTION("True condition")
            mtpl_buffer input = { "foo foo bar" };
            res = mtpl_generator_startsw(&allocs, &input, NULL, NULL, &buf);
            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(strcmp(out, "#t") == 0);
        END_SECTION
        
        SECTION("False condition")
            mtpl_buffer input = { "bar foo bar" };
            res = mtpl_generator_startsw(&allocs, &input, NULL, NULL, &buf);
            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(strcmp(out, "#f") == 0);
        END_SECTION
    END_SECTION

    SECTION("endsw")
        SECTION("True condition")
            mtpl_buffer input = { "foo foo bar" };
            res = mtpl_generator_startsw(&allocs, &input, NULL, NULL, &buf);
            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(strcmp(out, "#t") == 0);
        END_SECTION
        
        SECTION("False condition")
            mtpl_buffer input = { "bar foo bar" };
            res = mtpl_generator_startsw(&allocs, &input, NULL, NULL, &buf);
            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(strcmp(out, "#f") == 0);
        END_SECTION
    END_SECTION

    SECTION("contains")
        SECTION("True condition")
            mtpl_buffer input = { "foo foo bar" };
            res = mtpl_generator_startsw(&allocs, &input, NULL, NULL, &buf);
            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(strcmp(out, "#t") == 0);
        END_SECTION
        
        SECTION("False condition")
            mtpl_buffer input = { "bar foo bar" };
            res = mtpl_generator_startsw(&allocs, &input, NULL, NULL, &buf);
            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(strcmp(out, "#f") == 0);
        END_SECTION
    END_SECTION

    SECTION("range")
        SECTION("Default step")
            mtpl_buffer input = { "2 6" };
            res = mtpl_generator_range(&allocs, &input, NULL, NULL, &buf);
            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(strcmp(out, "2;3;4;5") == 0);
        END_SECTION

        SECTION("Negative step")
            mtpl_buffer input = { "6 0 -1.5" };
            res = mtpl_generator_range(&allocs, &input, NULL, NULL, &buf);
            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(strcmp(out, "6;4.5;3;1.5") == 0);
        END_SECTION
    END_SECTION

    SECTION("len")
        SECTION("Empty list")
            mtpl_buffer input = { "" };
            res = mtpl_generator_len(&allocs, &input, NULL, NULL, &buf);
            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(strcmp(out, "0") == 0);
        END_SECTION

        SECTION("Single element")
            mtpl_buffer input = { "foo" };
            res = mtpl_generator_len(&allocs, &input, NULL, NULL, &buf);
            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(strcmp(out, "1") == 0);
        END_SECTION

        SECTION("Populated list")
            mtpl_buffer input = { "foo;bar;baz;qux" };
            res = mtpl_generator_len(&allocs, &input, NULL, NULL, &buf);
            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(strcmp(out, "4") == 0);
        END_SECTION
    END_SECTION
    
    SECTION("element")
        SECTION("Index out of range")
            mtpl_buffer input = { "foo;bar;baz;qux 6" };
            res = mtpl_generator_element(&allocs, &input, NULL, NULL, &buf);
            REQUIRE(res == MTPL_ERR_UNKNOWN_KEY);
        END_SECTION
        
        SECTION("Index in range")
            mtpl_buffer input = { "foo;bar;baz;qux 2" };
            res = mtpl_generator_element(&allocs, &input, NULL, NULL, &buf);
            REQUIRE(res == MTPL_SUCCESS);
            REQUIRE(strcmp(out, "baz") == 0);
        END_SECTION
    END_SECTION

    mtpl_htable_free(&allocs, gens);
    mtpl_htable_free(&allocs, props);
END_FIXTURE

int main(void) {
    return RUN_TEST(generators);
}

