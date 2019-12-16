#pragma once

#include <mintpl/common.h>
#include <mintpl/hashtable.h>

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

