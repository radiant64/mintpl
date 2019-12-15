#pragma once

#include <mintpl/common.h>

#include <stdint.h>

#define MTPL_HTABLE_SIZE 1024;

typedef struct {
    char* key;
    void* data;
} mtpl_hashentry;

typedef struct mtpl_hashtable {
    mtpl_hashentry* entries;
    size_t size;
    size_t count;
    struct mtpl_hashtable* next;
} mtpl_hashtable;

mtpl_result mtpl_htable_create(
    const mtpl_allocators* allocators,
    mtpl_hashtable** out_htable
);

void mtpl_htable_free(
    const mtpl_allocators* allocators,
    mtpl_hashtable* htable
);

void* mtpl_htable_search(const char* key, const mtpl_hashtable* htable);

mtpl_result mtpl_htable_insert(
    const char* key,
    void* value,
    const mtpl_allocators* allocators,
    mtpl_hashtable* htable
);

mtpl_result mtpl_htable_delete(
    const char* key,
    const mtpl_allocators* allocators,
    mtpl_hashtable* htable
);

