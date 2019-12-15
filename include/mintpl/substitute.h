#pragma once

#include <mintpl/common.h>

mtpl_result mtpl_substitute(
    const char* source,
    mtpl_descriptors* descriptors,
    mtpl_properties* properties,
    mtpl_buffer* out_buffer
);

mtpl_result mtpl_custom_alloc_substitute(
    const char* source,
    const mtpl_allocators* allocators,
    mtpl_descriptors* descriptors,
    mtpl_properties* properties,
    mtpl_buffer* out_buffer
);

