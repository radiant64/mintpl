#pragma once

#include <mintpl/buffers.h>
#include <mintpl/common.h>
#include <mintpl/hashtable.h>

#ifdef _cplusplus
extern "C" {
#endif

typedef mtpl_result(*mtpl_generator)(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
);

mtpl_result mtpl_generator_copy(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
);

mtpl_result mtpl_generator_replace(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
);

mtpl_result mtpl_generator_for(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
);

mtpl_result mtpl_generator_if(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
);

#ifdef _cplusplus
}
#endif

