#include <mintpl/descriptor.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static int mtpl_lookup_compare(const void* a, const void* b) {
    return strcmp(a, *(char**) b);
}

mtpl_result mtpl_lookup(
    const char* name,
    const mtpl_descriptors* descriptors,
    mtpl_generator* out_generator
) {
    const char** found = bsearch(
        name,
        descriptors->names,
        descriptors->count,
        sizeof(char**),
        mtpl_lookup_compare
    );
    if (!found) {
        return MTPL_ERR_UNKNOWN_KEY;
    }

    uintptr_t index = (
        ((uintptr_t) found - (uintptr_t) descriptors->names) / sizeof(char**)
    );
    *out_generator = descriptors->generators[index];
    return MTPL_SUCCESS;
}

mtpl_result mtpl_insert_descriptor(
    const char* name,
    mtpl_generator generator,
    const mtpl_allocators* allocators,
    mtpl_descriptors* descriptors
) {
    size_t index = 0;
    for (
        ;
        index < descriptors->count 
            && strcmp(name, descriptors->names[index]) > 0;
        ++index
    ) { }

    if (descriptors->count + 1 >= descriptors->capacity) {
        MTPL_REALLOC_CHECKED(
            allocators,
            descriptors->names,
            descriptors->capacity * sizeof(char*) * 2
        );
        MTPL_REALLOC_CHECKED(
            allocators,
            descriptors->generators,
            descriptors->capacity * sizeof(mtpl_generator) * 2
        );
        descriptors->capacity *= 2;
    }
    
    if (index < descriptors->count) {
        memmove(
            &(descriptors->names[index + 1]),
            &(descriptors->names[index]),
            sizeof(char**) * (descriptors->count - index)
        );
        memmove(
            &(descriptors->generators[index + 1]),
            &(descriptors->generators[index]),
            sizeof(mtpl_generator) * (descriptors->count - index)
        );
    }

    size_t len = strlen(name);
    descriptors->names[index] = allocators->malloc(len + 1);
    if (!descriptors->names[index]) {
        return MTPL_ERR_MEMORY;
    }

    memcpy(descriptors->names[index], name, len + 1);
    descriptors->generators[index] = generator;
    
    descriptors->count++;
    return MTPL_SUCCESS;
}

mtpl_result mtpl_create_descriptors(const mtpl_allocators* allocators,
        mtpl_descriptors** out_descriptors) {
    *out_descriptors = allocators->malloc(sizeof(mtpl_descriptors));
    if (!*out_descriptors) {
        return MTPL_ERR_MEMORY;
    }
    (*out_descriptors)->names = allocators->malloc(
        sizeof(char*) * MTPL_INITIAL_DESCRIPTORS
    );
    if (!(*out_descriptors)->names) {
        return MTPL_ERR_MEMORY;
    }
    (*out_descriptors)->generators = allocators->malloc(
        sizeof(mtpl_generator) * MTPL_INITIAL_DESCRIPTORS
    );
    if (!(*out_descriptors)->generators) {
        return MTPL_ERR_MEMORY;
    }
    (*out_descriptors)->count = 0;
    (*out_descriptors)->capacity = MTPL_INITIAL_DESCRIPTORS;
    return MTPL_SUCCESS;
}

