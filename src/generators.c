#include <mintpl/generators.h>
#include <mintpl/substitute.h>

#include <stdbool.h>
#include <string.h>

inline static bool is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static const char* trim_whitespace(const char* text) {
    while (*text && is_whitespace(*text)) {
        text++;
    }

    return text;
}

static size_t word_length(const char* text, char delimiter) {
    text = trim_whitespace(text);
    size_t i = 0;

    while (
        text[i] 
        && delimiter == 0 ? !is_whitespace(text[i]) : text[i] != delimiter
    ) {
        i++;
    }

    return i++;
}

static const char* read_word(
    const char* text,
    char delimiter,
    size_t max_len,
    char* out_word
) {
    text = trim_whitespace(text);
    size_t i = 0;

    while (
        text[i] 
        && delimiter == 0 ? !is_whitespace(text[i]) : text[i] != delimiter
        && i < max_len - 1
    ) {
        out_word[i] = text[i];
        i++;
    }
    out_word[i] = '\0';

    return trim_whitespace(&text[++i]);
}

mtpl_result mtpl_generator_copy(
    const char* arg,
    const mtpl_allocators* allocators,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out_buffer
) {
    return mtpl_buffer_print(arg, allocators, out_buffer);
}

mtpl_result mtpl_generator_replace(
    const char* arg,
    const mtpl_allocators* allocators,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out_buffer
) {
    const char* value = mtpl_htable_search(arg, properties);
    if (!value) {
        return MTPL_ERR_UNKNOWN_KEY;
    }
    
    return mtpl_buffer_print(value, allocators, out_buffer);
}

mtpl_result mtpl_generator_for(
    const char* arg,
    const mtpl_allocators* allocators,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out_buffer
) {
    mtpl_result result = MTPL_SUCCESS;
    char variable[256];
    char item[256];
    size_t list_len = word_length(arg, 0) + 1;
    char* list = allocators->malloc(list_len);
    const char* list_cursor = list;
    
    arg = read_word(arg, 0, list_len, list);
    arg = read_word(arg, 0, 256, variable);

    while (*list_cursor) {
        const char* list_post = read_word(list_cursor, ';', 256, item);
        result = mtpl_htable_insert(
           variable,
           item,
           list_post - list_cursor,
           allocators,
           properties
        );
        if (result != MTPL_SUCCESS) {
            goto cleanup_list;
        }
        list_cursor = list_post;

        result = mtpl_substitute(
            arg,
            allocators,
            generators,
            properties,
            out_buffer
        );
        if (result != MTPL_SUCCESS) {
            goto cleanup_list;
        }
    }

cleanup_list:
    allocators->free(list);
    return result;
}

