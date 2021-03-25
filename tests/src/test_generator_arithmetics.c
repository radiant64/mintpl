#include "testdrive.h"

#include <mintpl/generators.h>

#include <string.h>

#define TEST_EXPR(INPUT, EXPECTED) {\
        mtpl_buffer input = { INPUT };\
        input.size = strlen(input.data);\
        res = mtpl_generator_arithmetics(&allocs, &input, NULL, NULL, &buf);\
        REQUIRE(res == MTPL_SUCCESS);\
        REQUIRE(strcmp(out, EXPECTED) == 0);\
    }

static const mtpl_allocators allocs = { malloc, realloc, free };

FIXTURE(generator_arithmetics, "Arithmetics generator")
    char out[32] = { 0 };
    mtpl_buffer buf = { .data = out, .size = 32 };
    mtpl_result res;

    SECTION("Addition")
        TEST_EXPR("1 + 2", "3")
    END_SECTION

    SECTION("Subtraction")
        TEST_EXPR("1 - 2", "-1")
    END_SECTION
    
    SECTION("Multiplication")
        TEST_EXPR("2 * 3", "6")
    END_SECTION
    
    SECTION("Division")
        TEST_EXPR("3 / 2", "1.5")
    END_SECTION
    
    SECTION("Modulo")
        TEST_EXPR("3 % 2", "1")
    END_SECTION
    
    SECTION("Power")
        TEST_EXPR("2 ^ 3", "8")
    END_SECTION
    
    SECTION("Negation")
        TEST_EXPR("-2", "-2")
    END_SECTION

    SECTION("Compound expression")
        TEST_EXPR("2 + 2 * 1.5", "5")
    END_SECTION
    
    SECTION("Parentheses")
        TEST_EXPR("-(2 + 2) * 1.5", "-6")
    END_SECTION
END_FIXTURE

int main(void) {
    return RUN_TEST(generator_arithmetics);
}

