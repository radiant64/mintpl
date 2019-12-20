#include <mintpl/substitute.h>

#include <mintpl/generators.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static mtpl_result lookup_generator(
    const mtpl_hashtable* generators,
    mtpl_readbuffer* source,
    mtpl_generator* out_generator
) {
    char gen_name[MTPL_GENERATOR_NAME_MAXLEN + 1] = { 0 };
    uint8_t gen_read_chars = 0;
    while (
        gen_read_chars <= MTPL_GENERATOR_NAME_MAXLEN 
            && source->data[++(source->cursor)] != '>'
    ) {
        gen_name[gen_read_chars++] = source->data[source->cursor];
    }

    if (source->data[source->cursor] != '>') {
        return MTPL_ERR_MALFORMED_NAME;
    }
    source->cursor++;
    gen_name[gen_read_chars] = '\0';
    *out_generator = *(void**) mtpl_htable_search(gen_name, generators);
    if (!out_generator) {
        return MTPL_ERR_UNKNOWN_KEY;
    }
    return MTPL_SUCCESS;
}

static mtpl_result perform_substitution(
    mtpl_generator generator,
    const mtpl_allocators* allocators,
    mtpl_readbuffer* source,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out_buffer
) {
    mtpl_generator sub_generator;
    mtpl_result result = MTPL_SUCCESS;
    mtpl_buffer arg_buffer = {
        .data = allocators->malloc(MTPL_DEFAULT_BUFSIZE),
        .size = MTPL_DEFAULT_BUFSIZE
    };
    memset(arg_buffer.data, 0, MTPL_DEFAULT_BUFSIZE);

    while (true) {
        switch (source->data[source->cursor]) {
        case '[':
            // Lookup generator name and begin new substitution.
            result = lookup_generator(generators, source, &sub_generator);
            if (result != MTPL_SUCCESS) {
                goto finish_substitution;
            }
            result = perform_substitution(
                sub_generator,
                allocators,
                source,
                generators,
                properties,
                &arg_buffer
            );
            if (result != MTPL_SUCCESS) {
                goto finish_substitution;
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
                goto finish_substitution;
            }
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
            arg_buffer.data[arg_buffer.cursor++]
                = source->data[source->cursor++];
            break;
        }
    }

finish_substitution:
    generator(arg_buffer.data, allocators, generators, properties, out_buffer);
    allocators->free(arg_buffer.data);
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

