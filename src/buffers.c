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

static size_t extract_length(
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
        && (
            (i > 0 && text[i - 1] == '\\')
            || (delimiter == 0 ? !is_whitespace(text[i]) : text[i] != delimiter)
        )
    ) {
        ++i;
    }

    return i;
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
    size_t len = strlen(&input->data[input->cursor]);
    if (output->cursor + len >= output->size) {
        MTPL_REALLOC_CHECKED(
            allocators,
            output->data,
            output->size * 2,
            return MTPL_ERR_MEMORY
        );
        output->size *= 2;
    }

    memcpy(&output->data[output->cursor], &input->data[input->cursor], len);
    output->cursor += len;
    output->data[output->cursor] = '\0';
    return MTPL_SUCCESS;
}

mtpl_result mtpl_buffer_extract(
    char delimiter,
    const mtpl_allocators* allocators,
    mtpl_buffer* input,
    mtpl_buffer* out
) {
    trim_whitespace(input);

    size_t len = extract_length(input, delimiter);
    if (out->size < out->cursor + len) {
        size_t size = out->size;
        do {
            size *= 2;
        } while (size < out->cursor + len);
        MTPL_REALLOC_CHECKED(
            allocators,
            out->data,
            size,
            return MTPL_ERR_MEMORY
        );
        out->size = size;
    }
    for (size_t i = 0; i < len; ++i) {
        const char c = input->data[input->cursor++];
        if (c != '\\') {
            out->data[out->cursor++] = c;
        }
    }
    if (input->data[input->cursor]) {
        input->cursor++;
    }
    out->data[out->cursor] = '\0';
    trim_whitespace(input);
    return MTPL_SUCCESS;
}

mtpl_result mtpl_buffer_extract_sub(
    const mtpl_allocators* allocators,
    const bool include_outer,
    mtpl_buffer* input,
    mtpl_buffer* out
) {
    trim_whitespace(input);
    const char opener = input->data[input->cursor];
    if (opener != '[' && opener != '{') {
        return mtpl_buffer_extract(0, allocators, input, out);
    }
    const char closer = opener == '[' ? ']' : '}';

    size_t level = 0;
    do {
        const char c = input->data[input->cursor];
        if (c == opener) {
            level++;
            if (!include_outer && level == 1) {
                input->cursor++;
                continue;
            }
        } else if (c == closer) {
            level--;
        } else if (c == '\\') {
            // Escape next character if not 0.
            if (!input->data[++(input->cursor)]) {
                return MTPL_ERR_SYNTAX;
            }
        }
        if (out->cursor >= out->size) {
            MTPL_REALLOC_CHECKED(
                allocators,
                out->data,
                out->size * 2,
                return MTPL_ERR_MEMORY
            );
            out->size *= 2;
        }
        if (include_outer || level > 0) {
            out->data[out->cursor++] = input->data[input->cursor++];
        }
    } while (level && input->data[input->cursor]);

    out->data[out->cursor] = '\0';

    return level > 1 ? MTPL_ERR_SYNTAX : MTPL_SUCCESS;
}

