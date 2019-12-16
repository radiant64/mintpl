#include <mintpl/substitute.h>

#include <mintpl/generators.h>

#include <stdbool.h>
#include <stdlib.h>

static mtpl_result mtpl_perform_substitution(
    mtpl_generator generator,
    const mtpl_allocators* allocators,
    mtpl_readbuffer* source,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out_buffer
) {
    mtpl_result result = MTPL_SUCCESS;
    mtpl_buffer arg_buffer = {
        .data = allocators->malloc(MTPL_DEFAULT_BUFSIZE),
        .size = MTPL_DEFAULT_BUFSIZE
    };
    char gen_name[MTPL_GENERATOR_NAME_MAXLEN + 1] = { 0 };
    uint8_t gen_read_chars = 0;
    mtpl_generator sub_generator;

    while (true) {
        switch (source->data[source->cursor]) {
        case '[':
            // Lookup generator name and begin new substitution.
            while (
                gen_read_chars <= MTPL_GENERATOR_NAME_MAXLEN 
                    && source->data[++(source->cursor)] != '>'
            ) {
                gen_name[gen_read_chars++] = source->data[source->cursor];
            }

            if (source->data[source->cursor] != '>') {
                result = MTPL_ERR_MALFORMED_NAME;
                goto finish_substitution;
            }
            gen_name[gen_read_chars] = '\0';
            sub_generator = mtpl_htable_search(gen_name, generators);
            if (!sub_generator) {
                result = MTPL_ERR_UNKNOWN_KEY;
                goto finish_substitution;
            }
            result = mtpl_perform_substitution(
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
            // Fallthrough
        case '\0':
            // End current substitution and return to parent.
            goto finish_substitution;
        case '\\':
            // Escape next character if not 0.
            if (!source->data[++source->cursor]) {
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
            arg_buffer.data[arg_buffer.cursor++] = source->cursor++;
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
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out_buffer
) {
    static const mtpl_allocators allocators = { malloc, realloc, free };
    mtpl_readbuffer buffer = {
        .data = source,
        .cursor = 0,
        .size = 0
    };
    return mtpl_perform_substitution(
        mtpl_generator_copy,
        &allocators,
        &buffer,
        generators,
        properties,
        out_buffer
    );
}

mtpl_result mtpl_custom_alloc_substitute(
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
    return mtpl_perform_substitution(
        mtpl_generator_copy,
        allocators,
        &buffer,
        generators,
        properties,
        out_buffer
    );
}

