#include <mintpl/substitute.h>

#include <mintpl/generators.h>

#include <stdbool.h>
#include <stdlib.h>

static mtpl_result mtpl_perform_substitution(
    mtpl_generator generator,
    const char* source,
    const mtpl_allocators* allocators,
    mtpl_descriptors* descriptors,
    mtpl_buffer* out_buffer
) {
    size_t read = 0;
    mtpl_buffer arg_buffer = {
        .data = allocators->malloc(MTPL_DEFAULT_BUFSIZE),
        .size = MTPL_DEFAULT_BUFSIZE
    };

    void* realloc_addr;
    while (true) {
        switch (*source) {
        case '<':
            // Lookup generator name and begin new substitution.
            //
            break;
        case '>':
        case '\0':
            // End current substitution and apply generator on it.
            break;
        case '\\':
            // Escape next character if not 0.
            if (!source[1]) {
                return MTPL_ERR_SYNTAX;
            }
            source++;
            read++;
            // Fall through.
        default:
            // Read into the arg buffer.
            if (arg_buffer.cursor >= arg_buffer.size) {
                MTPL_REALLOC_CHECKED(
                    allocators,
                    arg_buffer.data,
                    arg_buffer.size * 2
                );
                arg_buffer.size *= 2;
            }
            arg_buffer.data[arg_buffer.cursor++] = *(source++);
            read++;
            break;
        }
        source += read;
        read = 0;
    }

}

mtpl_result mtpl_substitute(
    const char* source,
    mtpl_descriptors* descriptors,
    mtpl_buffer* out_buffer
) {
    static const mtpl_allocators allocators = { malloc, realloc, free };
    return mtpl_perform_substitution(
        mtpl_generator_copy,
        source,
        &allocators,
        descriptors,
        out_buffer
    );
}

mtpl_result mtpl_custom_alloc_substitute(
    const char* source,
    const mtpl_allocators* allocators,
    mtpl_descriptors* descriptors,
    mtpl_buffer* out_buffer
) {
    return mtpl_perform_substitution(
        mtpl_generator_copy,
        source,
        allocators,
        descriptors,
        out_buffer
    );
}

