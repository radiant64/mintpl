#pragma once

#define MTPL_HTABLE_SIZE 16384

typedef struct {
    
} mtpl_hashtable;

void* mtpl_htable_search(const mtpl_hashtable* htable, const char* key);

mtpl_result mtpl_htable_insert(
    const mtpl_hashtable* htable,
    const char* key,
    const char* value
);

