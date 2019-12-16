#pragma once

#include <mintpl/common.h>
#include <mintpl/hashtable.h>

mtpl_result mtpl_substitute(
    const char* source,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out_buffer
);

mtpl_result mtpl_custom_alloc_substitute(
    const char* source,
    const mtpl_allocators* allocators,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out_buffer
);

