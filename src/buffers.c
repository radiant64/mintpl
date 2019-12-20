#include <mintpl/buffers.h>

#include <stdbool.h>
#include <string.h>

inline static bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static void trim_whitespace(mtpl_buffer* input) {
    while (
        input->data[input->cursor] 
        && is_whitespace(input->data[input->cursor])
    ) {
        input->cursor++;
    }
}

static size_t extract_word_length(
    const mtpl_buffer* input,
    char delimiter
) {
    const char* text = &input->data[input->cursor];
    while (*text && is_whitespace(*text)) {
        text++;
    }
    size_t i = 0;

    while (
        text[i] 
        && (delimiter == 0 ? !is_whitespace(text[i]) : text[i] != delimiter)
    ) {
        ++i;
    }

    return ++i;
}

mtpl_result mtpl_buffer_create(
    const mtpl_allocators* allocators,
    size_t size,
    mtpl_buffer** buffer
) {
    *buffer = allocators->malloc(sizeof(mtpl_buffer));
    if (!*buffer) {
        return MTPL_ERR_MEMORY;
    }
    (*buffer)->data = allocators->malloc(size);
    if (!(*buffer)->data) {
        return MTPL_ERR_MEMORY;
    }
    (*buffer)->cursor = 0;
    (*buffer)->size = size;

    return MTPL_SUCCESS;
}

mtpl_result mtpl_buffer_free(
    const mtpl_allocators* allocators,
    mtpl_buffer* buffer
) {
    allocators->free(buffer->data);
    allocators->free(buffer);
}

mtpl_result mtpl_buffer_print(
    const mtpl_buffer* input,
    const mtpl_allocators* allocators,
    mtpl_buffer* output
) {
    size_t len = strlen(input->data);
    if (output->cursor + len >= output->size) {
        MTPL_REALLOC_CHECKED(
            allocators,
            output->data,
            output->size * 2,
            return MTPL_ERR_MEMORY
        );
        output->size *= 2;
    }

    memcpy(&output->data[output->cursor], input->data, len);
    output->cursor += len;
    return MTPL_SUCCESS;
}

mtpl_result mtpl_buffer_extract_word(
    char delimiter,
    const mtpl_allocators* allocators,
    mtpl_buffer* input,
    mtpl_buffer* out
) {
    size_t i = 0;
    trim_whitespace(input);

    size_t len = extract_word_length(input, delimiter);
    if (out->size < len) {
        size_t size = out->size;
        do {
            size *= 2;
        } while (size < len);
        MTPL_REALLOC_CHECKED(
            allocators,
            out->data,
            size,
            return MTPL_ERR_MEMORY
        );
        out->size = size;
    }
    memcpy(out->data, &input->data[input->cursor], len - 1);
    out->data[len - 1] = '\0';
    input->cursor += len;
    trim_whitespace(input);
    return MTPL_SUCCESS;
}

