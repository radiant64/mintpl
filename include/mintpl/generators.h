#pragma once

#include <mintpl/buffers.h>
#include <mintpl/common.h>
#include <mintpl/hashtable.h>

#ifdef _cplusplus
extern "C" {
#endif

typedef mtpl_result(*mtpl_generator)(
    const char* arg,
    const mtpl_allocators* allocators,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out_buffer
);

mtpl_result mtpl_generator_copy(
    const char* arg,
    const mtpl_allocators* allocators,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out_buffer
);

mtpl_result mtpl_generator_replace(
    const char* arg,
    const mtpl_allocators* allocators,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out_buffer
);

mtpl_result mtpl_generator_for(
    const char* arg,
    const mtpl_allocators* allocators,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out_buffer
);

#ifdef _cplusplus
}
#endif

