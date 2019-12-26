#pragma once

#include <mintpl/buffers.h>
#include <mintpl/common.h>
#include <mintpl/generators.h>
#include <mintpl/hashtable.h>
#include <mintpl/version.h>

#ifdef _cplusplus
extern "C" {
#endif

typedef struct {
    const mtpl_allocators* allocators;
    mtpl_hashtable* generators;
    mtpl_hashtable* properties;
    mtpl_buffer* output;
} mtpl_context;

mtpl_result mtpl_init(mtpl_context** out_context);

mtpl_result mtpl_init_custom_alloc(
    const mtpl_allocators* allocators,
    mtpl_context** out_context
);

void mtpl_free(mtpl_context* context);

mtpl_result mtpl_set_generator(
    const char* name,
    mtpl_generator generator,
    mtpl_context* context
);

mtpl_result mtpl_set_property(
    const char* name,
    const char* value,
    mtpl_context* context
);

mtpl_result mtpl_parse_template(const char* source, mtpl_context* context);

#ifdef _cplusplus
}
#endif

