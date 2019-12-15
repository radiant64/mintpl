#pragma once

#include <mintpl/common.h>

mtpl_result mtpl_generator_copy(
    const char* arg,
    const mtpl_allocators* allocators,
    mtpl_descriptors* descriptors,
    mtpl_buffer* out_buffer
);

mtpl_result mtpl_generator_replace(
    const char* arg,
    const mtpl_allocators* allocators,
    mtpl_descriptors* descriptors,
    mtpl_buffer* out_buffer
);

