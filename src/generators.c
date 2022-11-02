#include <mintpl/generators.h>
#include <mintpl/substitute.h>

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
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

mtpl_result mtpl_generator_copy_strip(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    const size_t len = strlen(arg->data);
    if (!len) {
        // Empty string, output nothing.
        return MTPL_SUCCESS;
    }

    // Strip space from beginning.
    size_t i;
    for (i = 0; i < len && is_whitespace(arg->data[i]); ++i);
    if (i == len) {
        // String contains only whitespace; output nothing.
        return MTPL_SUCCESS;
    }
    arg->cursor = i;

    // Strip space from end.
    for (i = len - 1; i && is_whitespace(arg->data[i]); --i);

    return mtpl_buffer_nprint(arg, allocators, out, i + 1 - arg->cursor);
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

mtpl_result mtpl_generator_has_prop(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    const mtpl_buffer value = { mtpl_htable_search(arg->data, properties) };
    out->data[out->cursor] = '#';
    out->data[out->cursor + 2] = '\0';
    if (value.data) {
        out->data[out->cursor + 1] = 't';
    } else {
        out->data[out->cursor + 1] = 'f';
    }
    out->cursor += 2;
    
    return MTPL_SUCCESS;
}

mtpl_result mtpl_generator_escape(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    do { 
        if (out->cursor >= out->size - 1) {
            MTPL_REALLOC_CHECKED(
                allocators,
                out->data,
                out->size * 2, 
                return MTPL_ERR_MEMORY;
            );
            out->size *= 2;
        }
        switch (arg->data[arg->cursor]) {
        case ' ':
            out->data[out->cursor++] = '\\';
            // Fallthrough.
        default:
            out->data[out->cursor++] = arg->data[arg->cursor++];
            break;
        }
    } while (arg->data[arg->cursor]);

    return MTPL_SUCCESS;
}

static mtpl_result let_prop(
    const mtpl_allocators* allocators,
    mtpl_generator impl,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    mtpl_result result;
    mtpl_buffer* variable;
    mtpl_buffer* value;
    
    result = mtpl_buffer_create(allocators, MTPL_DEFAULT_BUFSIZE, &variable);
    if (result != MTPL_SUCCESS) {
        return result;
    }
    result = mtpl_buffer_create(allocators, MTPL_DEFAULT_BUFSIZE, &value);
    if (result != MTPL_SUCCESS) {
        goto cleanup_variable;
    }

    result = mtpl_buffer_extract(0, allocators, arg, variable);
    if (result != MTPL_SUCCESS) {
        goto cleanup_value;
    }
    result = impl(allocators, arg, generators, properties, value);
        
    if (result != MTPL_SUCCESS) {
        goto cleanup_value;
    }

    result = mtpl_htable_insert(
        variable->data,
        value->data,
        strlen(value->data) + 1,
        allocators,
        properties
    );

cleanup_value:
    mtpl_buffer_free(allocators, value);
cleanup_variable:
    mtpl_buffer_free(allocators, variable);

    return result;
}

static mtpl_result do_subst(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    return mtpl_substitute(
        &arg->data[arg->cursor],
        allocators,
        generators,
        properties,
        out
    );
}

mtpl_result mtpl_generator_let(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    return let_prop(allocators, do_subst, arg, generators, properties, out);
}

mtpl_result mtpl_generator_macro(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    return let_prop(
        allocators,
        mtpl_generator_copy,
        arg,
        generators,
        properties,
        out
    );
}

mtpl_result mtpl_generator_expand(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    mtpl_result res;
    mtpl_buffer* name;
    mtpl_buffer* arglist;
    mtpl_buffer* body;
    mtpl_buffer* param;
    mtpl_buffer* value;
    
    res = mtpl_buffer_create(allocators, MTPL_DEFAULT_BUFSIZE, &name);
    if (res!= MTPL_SUCCESS) {
        return res;
    }
    res = mtpl_buffer_create(allocators, MTPL_DEFAULT_BUFSIZE, &arglist);
    if (res!= MTPL_SUCCESS) {
        goto cleanup_name;
    }
    res = mtpl_buffer_create(allocators, MTPL_DEFAULT_BUFSIZE, &body);
    if (res != MTPL_SUCCESS) {
        goto cleanup_arglist;
    }
    res = mtpl_buffer_create(allocators, MTPL_DEFAULT_BUFSIZE, &param);
    if (res != MTPL_SUCCESS) {
        goto cleanup_body;
    }
    res = mtpl_buffer_create(allocators, MTPL_DEFAULT_BUFSIZE, &value);
    if (res != MTPL_SUCCESS) {
        goto cleanup_param;
    }

    res = mtpl_buffer_extract(0, allocators, arg, name);
    if (res != MTPL_SUCCESS) {
        goto cleanup_value;
    }
    mtpl_buffer def = { mtpl_htable_search(name->data, properties) };
    if (!def.data) {
        goto cleanup_value;
    }

    res = mtpl_buffer_extract(0, allocators, &def, arglist);
    if (res != MTPL_SUCCESS) {
        goto cleanup_value;
    }
    
    mtpl_hashtable* scope = NULL;
    res = mtpl_htable_create(allocators, &scope);
    if (res != MTPL_SUCCESS) {
        goto cleanup_value;
    }
    scope->next = properties;
    arglist->cursor = 0;
    while (arglist->data[arglist->cursor]) {
        res = mtpl_buffer_extract(';', allocators, arglist, param);
        if (res != MTPL_SUCCESS) {
            goto cleanup_scope;
        }
        res = mtpl_buffer_extract(0, allocators, arg, value);
        if (res != MTPL_SUCCESS) {
            goto cleanup_scope;
        }
        size_t len = strlen(value->data) + 1;
        res = mtpl_htable_insert(
           param->data,
           value->data,
           len,
           allocators,
           scope
        );
        param->cursor = 0;
        value->cursor = 0;
        if (res != MTPL_SUCCESS) {
            goto cleanup_scope;
        }
    }

    res = mtpl_substitute(
        &def.data[def.cursor],
        allocators,
        generators,
        scope,
        out
    );
    scope->next = NULL;

cleanup_scope:
    mtpl_htable_free(allocators, scope);
cleanup_value:
    mtpl_buffer_free(allocators, value);
cleanup_param:
    mtpl_buffer_free(allocators, param);
cleanup_body:
    mtpl_buffer_free(allocators, body);
cleanup_arglist:
    mtpl_buffer_free(allocators, arglist);
cleanup_name:
    mtpl_buffer_free(allocators, name);

    return res;
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
    result = mtpl_buffer_create(allocators, MTPL_DEFAULT_BUFSIZE, &list);
    if (result != MTPL_SUCCESS) {
        goto cleanup_item;
    }
    
    result = mtpl_buffer_extract(0, allocators, arg, list);
    if (result != MTPL_SUCCESS) {
        goto cleanup_list;
    }
    list->cursor = 0;
    result = mtpl_buffer_extract(0, allocators, arg, variable);
    if (result != MTPL_SUCCESS) {
        goto cleanup_list;
    }
    variable->cursor = 0;

    mtpl_hashtable* scope = NULL;
    result = mtpl_htable_create(allocators, &scope);
    if (result != MTPL_SUCCESS) {
        goto cleanup_list;
    }
    scope->next = properties;
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
           scope
        );
        item->cursor = 0;
        if (result != MTPL_SUCCESS) {
            break;
        }

        result = mtpl_substitute(
            &arg->data[arg->cursor],
            allocators,
            generators,
            scope,
            out
        );
        if (result != MTPL_SUCCESS) {
            break;
        }
    }
    scope->next = NULL;

    mtpl_htable_free(allocators, scope);
cleanup_list:
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
    mtpl_result res;
    if (arg->data[0] != '#' || !is_whitespace(arg->data[2])) {
        return MTPL_ERR_SYNTAX;
    }

    mtpl_buffer* expr;
    res = mtpl_buffer_create(allocators, MTPL_DEFAULT_BUFSIZE, &expr);
    if (res != MTPL_SUCCESS) {
        return res;
    }
    arg->cursor = 3;
    res = mtpl_buffer_extract_sub(allocators, true, arg, expr);
    if (res != MTPL_SUCCESS) {
        goto cleanup;
    }

    switch (arg->data[1]) {
    case 't':
        break;
    case 'f':
        expr->cursor = 0;
        res = mtpl_buffer_extract_sub(allocators, true, arg, expr);
        if (res != MTPL_SUCCESS) {
            res = MTPL_SUCCESS;
            goto cleanup;
        }
        break;
    default:
        res = MTPL_ERR_SYNTAX;
        goto cleanup;
    }
    res = mtpl_substitute(
        expr->data,
        allocators,
        generators,
        properties,
        out
    );

cleanup:
    mtpl_buffer_free(allocators, expr);

    return res;
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

static mtpl_result generator_cmp(
    const mtpl_allocators* allocators,
    bool(*compare)(const mtpl_buffer* a, const mtpl_buffer* b),
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
    
    result = mtpl_buffer_extract_sub(allocators, true, arg, sub);
    if (result != MTPL_SUCCESS) {
        goto cleanup_sub_gen_1;
    }
    result = mtpl_substitute(
        sub->data,
        allocators,
        generators,
        properties,
        sub_gen[0]
    );
    if (result != MTPL_SUCCESS) {
        goto cleanup_sub_gen_1;
    }
    sub_gen[0]->data[sub_gen[0]->cursor] = '\0';
    sub->cursor = 0;
    result = mtpl_buffer_extract_sub(allocators, true, arg, sub);
    if (result != MTPL_SUCCESS) {
        goto cleanup_sub_gen_1;
    }
    result = mtpl_substitute(
        sub->data,
        allocators,
        generators,
        properties,
        sub_gen[1]
    );
    if (result != MTPL_SUCCESS) {
        goto cleanup_sub_gen_1;
    }
    sub_gen[1]->data[sub_gen[1]->cursor] = '\0';

    state.data = compare(sub_gen[0], sub_gen[1]) ? "#t" : "#f";
    result = mtpl_buffer_print(&state, allocators, out);

cleanup_sub_gen_1:
    mtpl_buffer_free(allocators, sub_gen[1]);
cleanup_sub_gen_0:
    mtpl_buffer_free(allocators, sub_gen[0]);
cleanup_sub:
    mtpl_buffer_free(allocators, sub);
    return result;
}

static bool equals(const mtpl_buffer* a, const mtpl_buffer* b) {
    return strcmp(a->data, b->data) == 0;
}

static bool greater(const mtpl_buffer* a, const mtpl_buffer* b) {
    return strcmp(a->data, b->data) > 0;
}

static bool less(const mtpl_buffer* a, const mtpl_buffer* b) {
    return strcmp(a->data, b->data) < 0;
}

static bool gteq(const mtpl_buffer* a, const mtpl_buffer* b) {
    return strcmp(a->data, b->data) >= 0;
}

static bool lteq(const mtpl_buffer* a, const mtpl_buffer* b) {
    return strcmp(a->data, b->data) <= 0;
}

mtpl_result mtpl_generator_equals(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    return generator_cmp(allocators, equals, arg, generators, properties, out); 
}

mtpl_result mtpl_generator_greater(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    return generator_cmp(allocators, greater, arg, generators, properties, out); 
}

mtpl_result mtpl_generator_less(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    return generator_cmp(allocators, less, arg, generators, properties, out); 
}

mtpl_result mtpl_generator_gteq(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    return generator_cmp(allocators, gteq, arg, generators, properties, out); 
}

mtpl_result mtpl_generator_lteq(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    return generator_cmp(allocators, lteq, arg, generators, properties, out); 
}

static mtpl_result gen_strcmp(
    const mtpl_allocators* allocators,
    bool(*compare)(const mtpl_buffer* a, const char* b),
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    mtpl_result res;
    mtpl_buffer* comparand;
    res = mtpl_buffer_create(allocators, MTPL_DEFAULT_BUFSIZE, &comparand);
    if (res != MTPL_SUCCESS) {
        return res;
    }
    res = mtpl_buffer_extract(0, allocators, arg, comparand);
    if (res != MTPL_SUCCESS) {
        goto cleanup;
    }

    out->data[out->cursor] = '#';
    out->data[out->cursor + 2] = '\0';
    const char* test = &(arg->data[arg->cursor]);
    if (compare(comparand, test)) {
        out->data[out->cursor + 1] = 't';
    } else {
        out->data[out->cursor + 1] = 'f';
    }
    out->cursor += 2;

cleanup:
    mtpl_buffer_free(allocators, comparand);

    return res;
}

static bool startsw(const mtpl_buffer* a, const char* b) {
    return strncmp(a->data, b, strlen(a->data)) == 0;
}

static bool endsw(const mtpl_buffer* a, const char* b) {
    const size_t alen = strlen(a->data);
    const size_t blen = strlen(b);
    if (blen < alen) {
        return false;
    }
    return strncmp(a->data, &(b[blen - alen]), alen) == 0;
}

static bool contains(const mtpl_buffer* a, const char* b) {
    return strstr(b, a->data) != NULL;
}

mtpl_result mtpl_generator_startsw(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    return gen_strcmp(allocators, startsw, arg, generators, properties, out);
}

mtpl_result mtpl_generator_endsw(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    return gen_strcmp(allocators, endsw, arg, generators, properties, out);
}

mtpl_result mtpl_generator_contains(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    return gen_strcmp(allocators, contains, arg, generators, properties, out);
}

mtpl_result mtpl_generator_range(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    errno = 0;
    char* start_cursor;
    char* end_cursor;
    char* step_cursor;
    double start = strtod(arg->data, &start_cursor);
    if (arg->data == start_cursor || errno) {
        return MTPL_ERR_SYNTAX;
    }
    double end = strtod(start_cursor, &end_cursor);
    if (start_cursor == end_cursor || errno) {
        return MTPL_ERR_SYNTAX;
    }
    double step = 1;
    if (*end_cursor) {
        step = strtod(end_cursor, &step_cursor);
        if (end_cursor == step_cursor || errno) {
            return MTPL_ERR_SYNTAX;
        }
        if (!step) {
            return MTPL_ERR_SYNTAX;
        }
    }

    char num_data[32] = { 0 };
    mtpl_buffer num = { num_data };
    snprintf(num_data, 32, "%g", start);
    mtpl_result res = mtpl_buffer_print(&num, allocators, out);
    if (res != MTPL_SUCCESS) {
        return res;
    }
    for (double i = start + step; (step > 0) ? i < end : i > end; i += step) {
        snprintf(num_data, 32, ";%g", i);
        mtpl_buffer_print(&num, allocators, out);
        if (res != MTPL_SUCCESS) {
            return res;
        }
    }

    return MTPL_SUCCESS;
}

mtpl_result mtpl_generator_len(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    size_t count = 0;
    if (arg->data[0]) {
        count = 1;
        for (; arg->data[arg->cursor]; arg->cursor++) {
            if (arg->data[arg->cursor] == '\\') {
                // Skip counting escaped characters.
                arg->cursor++;
            } else if (arg->data[arg->cursor] == ';') {
                count++;
            }
        }
    }
    
    char num_data[32] = { 0 };
    mtpl_buffer num = { num_data };
    snprintf(num_data, 32, "%ld", count);
    return mtpl_buffer_print(&num, allocators, out);
}

mtpl_result mtpl_generator_element(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    mtpl_result res; 
    mtpl_buffer* list;
    mtpl_buffer* value;
    res = mtpl_buffer_create(allocators, MTPL_DEFAULT_BUFSIZE, &list);
    if (res != MTPL_SUCCESS) {
        return res;
    }
    res = mtpl_buffer_create(allocators, MTPL_DEFAULT_BUFSIZE, &value);
    if (res != MTPL_SUCCESS) {
        goto cleanup_list;
    }

    res = mtpl_buffer_extract(0, allocators, arg, list);
    if (res != MTPL_SUCCESS) {
        goto cleanup_value;
    }

    char* index_cursor;
    size_t index = strtol(&arg->data[arg->cursor], &index_cursor, 10);
    if (&arg->data[arg->cursor] == index_cursor || errno) {
        res = MTPL_ERR_SYNTAX;
        goto cleanup_value;
    }

    size_t count = 0;
    list->cursor = 0;
    while (count < index && list->data[list->cursor]) {
        if (list->data[list->cursor] == '\\') {
            // Skip counting escaped characters.
            list->cursor++;
        } else if (list->data[list->cursor] == ';') {
            count++;
        }
        list->cursor++;
    } 

    if (!list->data[list->cursor]) {
        res = MTPL_ERR_UNKNOWN_KEY;
        goto cleanup_value;
    }

    res = mtpl_buffer_extract(';', allocators, list, value);
    if (res != MTPL_SUCCESS) {
        goto cleanup_value;
    }
    value->cursor = 0;

    res = mtpl_buffer_print(value, allocators, out);

cleanup_value:
    mtpl_buffer_free(allocators, value);
cleanup_list:
    mtpl_buffer_free(allocators, list);

    return res;
}

