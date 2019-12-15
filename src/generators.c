#include <mintpl/generators.h>

#include <string.h>

mtpl_result mtpl_generator_copy(
    const char* arg,
    const mtpl_allocators* allocators,
    mtpl_descriptors* descriptors,
    mtpl_buffer* out_buffer
) {
    size_t len = strlen(arg);
    if (out_buffer->cursor + len >= out_buffer->size) {
        MTPL_REALLOC_CHECKED(
            allocators,
            out_buffer->data,
            out_buffer->size * 2
        );
        out_buffer->size *= 2;
    }

    memcpy(&out_buffer->data[out_buffer->cursor], arg, len);
    out_buffer->cursor += len;
    return MTPL_SUCCESS;
}

mtpl_result mtpl_generator_replace(
    const char* arg,
    const mtpl_allocators* allocators,
    mtpl_descriptors* descriptors,
    mtpl_buffer* out_buffer
) {
    size_t len = strlen(arg);
    if (out_buffer->cursor + len >= out_buffer->size) {
        MTPL_REALLOC_CHECKED(
            allocators,
            out_buffer->data,
            out_buffer->size * 2
        );
        out_buffer->size *= 2;
    }

    memcpy(&out_buffer->data[out_buffer->cursor], arg, len);
    out_buffer->cursor += len;
    return MTPL_SUCCESS;
}

