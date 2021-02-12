#include "testdrive.h"

#include <mintpl/hashtable.h>

static const mtpl_allocators allocs = { malloc, realloc, free };

FIXTURE(hashtable, "Hashtable")
    char input[] = "foo bar";

    mtpl_hashtable* htable;
    mtpl_result res = mtpl_htable_create(&allocs, &htable);

    REQUIRE(res == MTPL_SUCCESS);
    REQUIRE(htable);
    REQUIRE(htable->entries);
    REQUIRE(htable->count == 0);
    
    SECTION("Insertion")
        res = mtpl_htable_insert("test", input, sizeof(input), &allocs, htable);
        REQUIRE(res == MTPL_SUCCESS);
        REQUIRE(htable->count == 1);

        char* found;

        SECTION("Search")
            found = mtpl_htable_search("test", htable);

            REQUIRE(strcmp(found, input) == 0);
            REQUIRE(found != input);
        END_SECTION

        SECTION("Deletion")
            res = mtpl_htable_delete("test", &allocs, htable);
            found = mtpl_htable_search("test", htable);
            
            REQUIRE(!found);
        END_SECTION
    END_SECTION

    mtpl_htable_free(&allocs, htable);
END_FIXTURE

int main(void) {
    return RUN_TEST(hashtable);
}

