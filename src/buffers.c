#include <mintpl/buffers.h>

mtpl_result mtpl_buffer_create(
    const mtpl_allocators* allocators,
    size_t size,
    mtpl_buffer** out_buffer
) {
    *out_buffer = allocators->malloc(size);
    if (!*out_buffer) {
        return MTPL_ERR_MEMORY;
    }

    return MTPL_SUCCESS;
}

mtpl_result mtpl_buffer_free(
    const mtpl_allocators* allocators,
    mtpl_buffer* buffer
) {
    allocators->free(buffer);
}

