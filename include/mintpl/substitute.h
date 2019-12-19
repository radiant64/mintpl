#pragma once

#include <mintpl/buffers.h>
#include <mintpl/common.h>
#include <mintpl/hashtable.h>

#ifdef _cplusplus
extern "C" {
#endif

mtpl_result mtpl_substitute(
    const char* source,
    const mtpl_allocators* allocators,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out_buffer
);

#ifdef _cplusplus
}
#endif

