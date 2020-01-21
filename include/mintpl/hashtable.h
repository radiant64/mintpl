#pragma once

#include <mintpl/common.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MTPL_HTABLE_SIZE 1024

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
    const void* value,
    size_t value_size,
    const mtpl_allocators* allocators,
    mtpl_hashtable* htable
);

mtpl_result mtpl_htable_delete(
    const char* key,
    const mtpl_allocators* allocators,
    mtpl_hashtable* htable
);

#ifdef __cplusplus
}
#endif

