#pragma once

#include <mintpl/common.h>

mtpl_result mtpl_lookup(
    const char* name,
    const mtpl_descriptors* descriptors,
    mtpl_generator* out_generator
);

mtpl_result mtpl_insert_descriptor(
    const char* name,
    mtpl_generator generator,
    const mtpl_allocators* allocators,
    mtpl_descriptors* descriptors
);

mtpl_result mtpl_create_descriptors(const mtpl_allocators* allocators,
        mtpl_descriptors** out_descriptors);

