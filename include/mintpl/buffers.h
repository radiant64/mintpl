#pragma once

#include <mintpl/common.h>
#include <stddef.h>

#ifdef _cplusplus
extern "C" {
#endif

typedef struct {
    char* data;
    size_t cursor;
    size_t size;
} mtpl_buffer;

typedef struct {
    const char* data;
    size_t cursor;
    size_t size;
} mtpl_readbuffer;

mtpl_result mtpl_buffer_create(
    const mtpl_allocators* allocators,
    size_t size,
    mtpl_buffer** out_buffer
);

mtpl_result mtpl_buffer_free(
    const mtpl_allocators* allocators,
    mtpl_buffer* buffer
);

#ifdef _cplusplus
}
#endif

