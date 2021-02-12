#include <mintpl/substitute.h>

#include <mintpl/generators.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static mtpl_result perform_substitution(
    mtpl_generator generator,
    const mtpl_allocators* allocators,
    mtpl_readbuffer* source,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out_buffer
) {
    mtpl_generator sub_generator;
    mtpl_result result;
    mtpl_buffer* gen_name;
    result = mtpl_buffer_create(allocators, MTPL_DEFAULT_BUFSIZE, &gen_name);
    if (result != MTPL_SUCCESS) {
        return result;
    }
    mtpl_buffer* arg_buffer;
    result = mtpl_buffer_create(allocators, MTPL_DEFAULT_BUFSIZE, &arg_buffer);
    if (result != MTPL_SUCCESS) {
        goto cleanup_gen_name;
    }
    memset(arg_buffer->data, 0, MTPL_DEFAULT_BUFSIZE);

    while (true) {
        switch (source->data[source->cursor]) {
        case '[':
            source->cursor++;
            // Lookup generator name and begin new substitution.
            result = mtpl_buffer_extract(
                '>',
                allocators,
                (mtpl_buffer*) source,
                gen_name
            );
            if (result != MTPL_SUCCESS) {
                goto cleanup_arg_buffer;
            }
            sub_generator = mtpl_htable_search(gen_name->data, generators);
            if (!sub_generator) {
                result = MTPL_ERR_UNKNOWN_KEY;
                goto cleanup_arg_buffer;
            }
            gen_name->cursor = 0;
            result = perform_substitution(
                *(void**) sub_generator,
                allocators,
                source,
                generators,
                properties,
                arg_buffer
            );
            if (result != MTPL_SUCCESS) {
                goto cleanup_arg_buffer;
            }
            break;
        case '{':
            result = mtpl_buffer_extract_sub(
                allocators,
                false,
                (mtpl_buffer*) source,
                arg_buffer
            );
            if (result != MTPL_SUCCESS) {
                goto cleanup_arg_buffer;
            }
            if (source->data[source->cursor]) {
                source->cursor++;
            }

            break;
        case ']':
            source->cursor++;
            // Fall through.
        case '\0':
            // End current substitution and return to parent.
            goto finish_substitution;
        case '\\':
            // Escape next character if not 0.
            if (!source->data[++(source->cursor)]) {
                result = MTPL_ERR_SYNTAX;
                goto cleanup_arg_buffer;
            }
            // Fall through.
        default:
            // Read into the arg buffer.
            if (arg_buffer->cursor >= arg_buffer->size) {
                MTPL_REALLOC_CHECKED(
                    allocators,
                    arg_buffer->data,
                    arg_buffer->size * 2, {
                        result = MTPL_ERR_MEMORY;
                        goto cleanup_arg_buffer;
                    }
                );
                arg_buffer->size *= 2;
            }
            arg_buffer->data[arg_buffer->cursor++]
                = source->data[source->cursor++];
            break;
        }
    }

finish_substitution:
    arg_buffer->cursor = 0;
    result = generator(
        allocators,
        arg_buffer,
        generators,
        properties,
        out_buffer
    );

cleanup_arg_buffer:
    mtpl_buffer_free(allocators, arg_buffer);
cleanup_gen_name:
    mtpl_buffer_free(allocators, gen_name);
    return result;
}

mtpl_result mtpl_substitute(
    const char* source,
    const mtpl_allocators* allocators,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out_buffer
) {
    mtpl_readbuffer buffer = {
        .data = source,
        .cursor = 0,
        .size = 0
    };
    return perform_substitution(
        mtpl_generator_copy,
        allocators,
        &buffer,
        generators,
        properties,
        out_buffer
    );
}

