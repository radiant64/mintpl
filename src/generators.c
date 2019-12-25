#include <mintpl/generators.h>
#include <mintpl/substitute.h>

#include <stdbool.h>
#include <string.h>

inline static bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

mtpl_result mtpl_generator_nop(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    return MTPL_SUCCESS;
}

mtpl_result mtpl_generator_copy(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    return mtpl_buffer_print(arg, allocators, out);
}

mtpl_result mtpl_generator_replace(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    const mtpl_buffer value = { mtpl_htable_search(arg->data, properties) };
    if (!value.data) {
        return MTPL_ERR_UNKNOWN_KEY;
    }
    
    return mtpl_buffer_print(&value, allocators, out);
}

mtpl_result mtpl_generator_for(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    mtpl_buffer* variable;
    mtpl_buffer* item;
    mtpl_buffer* list;
    mtpl_result result = mtpl_buffer_create(
        allocators,
        MTPL_DEFAULT_BUFSIZE,
        &variable
    );
    if (result != MTPL_SUCCESS) {
        return result;
    }
    result = mtpl_buffer_create(allocators, MTPL_DEFAULT_BUFSIZE, &item);
    if (result != MTPL_SUCCESS) {
        goto cleanup_variable;
    }
    mtpl_buffer_create(allocators, MTPL_DEFAULT_BUFSIZE, &list);
    if (result != MTPL_SUCCESS) {
        goto cleanup_item;
    }
    
    result = mtpl_buffer_extract(0, allocators, arg, list);
    if (result != MTPL_SUCCESS) {
        goto cleanup_item;
    }
    list->cursor = 0;
    result = mtpl_buffer_extract(0, allocators, arg, variable);
    if (result != MTPL_SUCCESS) {
        goto cleanup_item;
    }
    variable->cursor = 0;

    while (list->data[list->cursor]) {
        result = mtpl_buffer_extract(';', allocators, list, item);
        if (result != MTPL_SUCCESS) {
            break;
        }
        size_t len = strlen(item->data) + 1;
        result = mtpl_htable_insert(
           variable->data,
           item->data,
           len,
           allocators,
           properties
        );
        item->cursor = 0;
        if (result != MTPL_SUCCESS) {
            break;
        }

        result = mtpl_substitute(
            &arg->data[arg->cursor],
            allocators,
            generators,
            properties,
            out
        );
        if (result != MTPL_SUCCESS) {
            break;
        }
    }

    mtpl_buffer_free(allocators, list);
cleanup_item:
    mtpl_buffer_free(allocators, item);
cleanup_variable:
    mtpl_buffer_free(allocators, variable);
    return result;
}

mtpl_result mtpl_generator_if(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    if (arg->data[0] != '#' || !is_whitespace(arg->data[2])) {
        return MTPL_ERR_SYNTAX;
    }

    switch (arg->data[1]) {
    case 't':
        arg->cursor = 3;
        return mtpl_substitute(
            &arg->data[arg->cursor],
            allocators,
            generators,
            properties,
            out
        );
    case 'f':
        return MTPL_SUCCESS;
    default:
        return MTPL_ERR_SYNTAX;
    }
}

mtpl_result mtpl_generator_not(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    if (arg->data[0] != '#' || (arg->data[2] && !is_whitespace(arg->data[2]))) {
        return MTPL_ERR_SYNTAX;
    }

    if (out->size < 2) {
        MTPL_REALLOC_CHECKED(
            allocators,
            out->data,
            2,
            return MTPL_ERR_MEMORY
        );
        out->size = 2;
    }

    switch (arg->data[1]) {
    case 't':
        out->data[1] = 'f';
        break;
    case 'f':
        out->data[1] = 't';
        break;
    default:
        return MTPL_ERR_SYNTAX;
    }

    out->data[0] = '#';
    out->data[2] = '\0';

    return MTPL_SUCCESS;
}

mtpl_result mtpl_generator_equals(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    mtpl_result result;
    mtpl_buffer state = { "#f" };
    mtpl_buffer* sub;
    mtpl_buffer* sub_gen[2];
    result = mtpl_buffer_create(allocators, MTPL_DEFAULT_BUFSIZE, &sub);
    if (result != MTPL_SUCCESS) {
        return result;
    }
    result = mtpl_buffer_create(allocators, MTPL_DEFAULT_BUFSIZE, &sub_gen[0]);
    if (result != MTPL_SUCCESS) {
        goto cleanup_sub;
    }
    result = mtpl_buffer_create(allocators, MTPL_DEFAULT_BUFSIZE, &sub_gen[1]);
    if (result != MTPL_SUCCESS) {
        goto cleanup_sub_gen_0;
    }
    
    int buf = 0;
    bool has_prev_value = false;
    while (arg->data[arg->cursor]) {
        sub->cursor = 0;
        result = mtpl_buffer_extract_sub(allocators, arg, sub);
        if (result != MTPL_SUCCESS) {
            goto cleanup_sub_gen_1;
        }
        result = mtpl_substitute(
            sub->data,
            allocators,
            generators,
            properties,
            sub_gen[buf]
        );
        sub_gen[buf]->data[sub_gen[buf]->cursor] = '\0';
        if (
            has_prev_value 
            && strcmp(sub_gen[buf]->data, sub_gen[buf ^ 1]->data) != 0
        ) {
            result = mtpl_buffer_print(&state, allocators, out);
            goto cleanup_sub_gen_1;
        }
        buf ^= 1;
        has_prev_value = true;
    }

    state.data = "#t";
    result = mtpl_buffer_print(&state, allocators, out);

cleanup_sub_gen_1:
    mtpl_buffer_free(allocators, sub_gen[1]);
cleanup_sub_gen_0:
    mtpl_buffer_free(allocators, sub_gen[0]);
cleanup_sub:
    mtpl_buffer_free(allocators, sub);
    return result;
}

