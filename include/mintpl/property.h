#pragma once

#include <mintpl/common.h>

mtpl_result mtpl_get_property(
    const char* name,
    const mtpl_properties* properties,
    const char** out_value
);

mtpl_result mtpl_set_property(
    const char* name,
    const char* value,
    const mtpl_allocators* allocators,
    mtpl_properties* properties
);

