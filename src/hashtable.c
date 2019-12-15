#include <mintpl/hashtable.h>

#include <stdbool.h>
#include <string.h>

// Jenkins' one-at-a-time hash.
static uint32_t calculate_hash(const char* key, uint32_t mod) {
    uint32_t hash = 0;
    while (*key) {
        hash += *key++;
        hash += hash << 10;
        hash ^= hash >> 6;
    }
    hash += hash << 3;
    hash ^= hash >> 11;
    hash += hash << 15;
    return hash % mod;
}

mtpl_result mtpl_htable_create(
    const mtpl_allocators* allocators,
    mtpl_hashtable** out_htable
) {
    out_htable = allocators->malloc(sizeof(mtpl_hashtable));
    if (!out_htable) {
        return MTPL_ERR_MEMORY;
    }
    out_htable->entries = allocators->malloc(
        sizeof(mtpl_hashentry) * MTPL_HTABLE_SIZE
    );
    if (!out_htable->entries) {
        return MTPL_ERR_MEMORY;
    }
    memset(out_htable->entries, 0, sizeof(mtpl_hashentry) * MTPL_HTABLE_SIZE);
    out_htable->size = MTPL_HTABLE_SIZE;
    out_htable->count = 0;
    out_htable->next = NULL;
    return MTPL_SUCCESS;
}

void mtpl_htable_free(
    const mtpl_allocators* allocators,
    mtpl_hashtable* htable
) {
    if (htable->next) {
        mtpl_htable_free(allocators, htable->next);
    }
    for (size_t i = 0; i < htable->size; ++i) {
        allocators->free(htable->entries[i].key);
    }
    allocators->free(htable->entries);
    allocators->free(htable);
}

void* mtpl_htable_search(const char* key, const mtpl_hashtable* htable) {
    uint32_t index = calculate_hash(key, htable->size);
    for (uint32_t i = 0; i < 16; ++i) {
        if (strcmp(htable->entries[index + i].key, key) == 0) {
            return htable->entries[index + i].data;
        }
    }
    if (htable->next) {
        return mtpl_htable_search(key, htable->next);
    }
    return NULL;
}

mtpl_result mtpl_htable_insert(
    const char* key,
    void* value,
    const mtpl_allocators* allocators,
    mtpl_hashtable* htable
) {
    uint32_t index = calculate_hash(key, htable->size);
    for (uint32_t i = 0; i < 16; ++i) {
        const char** entrykey = &(htable->entries[index + i].key); 
        if (!*entrykey || strcmp(*entrykey, key) == 0) {
            size_t len = strlen(key) + 1;
            *entrykey = allocators->malloc(len);
            if (!*entrykey) {
                return MTPL_ERR_MEMORY;
            }
            memcpy(*entrykey, key, len);
            htable->count++;
            return MTPL_SUCCESS;
        }
    }
    if (!htable->next) {
        mtpl_result result = mtpl_htable_create(allocators, &(htable->next));
        if (result != MTPL_SUCCESS) {
            return result;
        }
    }
    return mtpl_htable_insert(key, value, allocators, htable->next);
}

mtpl_result mtpl_htable_delete(
    const char* key,
    const mtpl_allocators* allocators,
    mtpl_hashtable* htable
) {
    uint32_t index = calculate_hash(key, htable->size);
    for (uint32_t i = 0; i < 16; ++i) {
        const char** entrykey = &(htable->entries[index + i].key); 
        if (strcmp(*entrykey, key) == 0) {
            allocators->free(*entrykey);
            *entrykey = NULL;
            htable->count--;
            return MTPL_SUCCESS;
        }
    }
    if (htable->next) {
        return mtpl_htable_delete(key, allocators, htable->next);
    }
    return MTPL_ERR_UNKNOWN_KEY;
}

