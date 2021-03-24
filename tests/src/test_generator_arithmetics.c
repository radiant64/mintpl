#include "testdrive.h"

#include <mintpl/generators.h>

#include <string.h>

static const mtpl_allocators allocs = { malloc, realloc, free };

FIXTURE(generator_arithmetics, "Arithmetics generator")
    char out[32] = { 0 };
    mtpl_buffer buf = { .data = out, .size = 32 };
    mtpl_result res;

    SECTION("Addition")
        mtpl_buffer input = { "1 + 2" };
        input.size = strlen(input.data);
        res = mtpl_generator_arithmetics(&allocs, &input, NULL, NULL, &buf);
        
        REQUIRE(res == MTPL_SUCCESS);
        REQUIRE(strcmp(out, "3") == 0);
    END_SECTION
END_FIXTURE

int main(void) {
    return RUN_TEST(generator_arithmetics);
}

