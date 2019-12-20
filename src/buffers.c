#include <mintpl/buffers.h>

#include <string.h>

mtpl_result mtpl_buffer_create(
    const mtpl_allocators* allocators,
    size_t size,
    mtpl_buffer** buffer
) {
    *buffer = allocators->malloc(size);
    if (!*buffer) {
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

mtpl_result mtpl_buffer_print(
    const char* text,
    const mtpl_allocators* allocators,
    mtpl_buffer* buffer
) {
    size_t len = strlen(text);
    if (buffer->cursor + len >= buffer->size) {
        MTPL_REALLOC_CHECKED(
            allocators,
            buffer->data,
            buffer->size * 2
        );
        buffer->size *= 2;
    }

    memcpy(&buffer->data[buffer->cursor], text, len);
    buffer->cursor += len;
    return MTPL_SUCCESS;
}

