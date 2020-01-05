#include <mintpl/generators.h>
#include <mintpl/mintpl.h>
#include <mintpl/substitute.h>
#include <stdlib.h>
#include <string.h>

static const mtpl_allocators allocators = { malloc, realloc, free };

static mtpl_result add_default_generators(
    const mtpl_allocators* allocators,
    mtpl_hashtable* generators
) {
    mtpl_generator nop = mtpl_generator_nop;
    mtpl_generator copy = mtpl_generator_copy;
    mtpl_generator replace = mtpl_generator_replace;
    mtpl_generator genfor = mtpl_generator_for;
    mtpl_generator genif = mtpl_generator_if;
    mtpl_generator not = mtpl_generator_not;
    mtpl_generator eq = mtpl_generator_equals;
    mtpl_generator gt = mtpl_generator_greater;
    mtpl_generator lt = mtpl_generator_less;
    mtpl_generator ge = mtpl_generator_gteq;
    mtpl_generator le = mtpl_generator_lteq;
    mtpl_result result = mtpl_htable_insert(
        "!",
        &nop,
        sizeof(mtpl_generator),
        allocators,
        generators
    );
    if (result != MTPL_SUCCESS) {
        return result;
    }
    result = mtpl_htable_insert(
        ":",
        &copy,
        sizeof(mtpl_generator),
        allocators,
        generators
    );
    if (result != MTPL_SUCCESS) {
        return result;
    }
    result = mtpl_htable_insert(
        "=",
        &replace,
        sizeof(mtpl_generator),
        allocators,
        generators
    );
    if (result != MTPL_SUCCESS) {
        return result;
    }
    result = mtpl_htable_insert(
        "for",
        &genfor,
        sizeof(mtpl_generator),
        allocators,
        generators
    );
    if (result != MTPL_SUCCESS) {
        return result;
    }
    result = mtpl_htable_insert(
        "if",
        &genif,
        sizeof(mtpl_generator),
        allocators,
        generators
    );
    if (result != MTPL_SUCCESS) {
        return result;
    }
    result = mtpl_htable_insert(
        "not",
        &not,
        sizeof(mtpl_generator),
        allocators,
        generators
    );
    if (result != MTPL_SUCCESS) {
        return result;
    }
    result = mtpl_htable_insert(
        "eq",
        &eq,
        sizeof(mtpl_generator),
        allocators,
        generators
    );
    if (result != MTPL_SUCCESS) {
        return result;
    }
    result = mtpl_htable_insert(
        "gt",
        &gt,
        sizeof(mtpl_generator),
        allocators,
        generators
    );
    if (result != MTPL_SUCCESS) {
        return result;
    }
    result = mtpl_htable_insert(
        "lt",
        &lt,
        sizeof(mtpl_generator),
        allocators,
        generators
    );
    if (result != MTPL_SUCCESS) {
        return result;
    }
    result = mtpl_htable_insert(
        "ge",
        &ge,
        sizeof(mtpl_generator),
        allocators,
        generators
    );
    if (result != MTPL_SUCCESS) {
        return result;
    }
    result = mtpl_htable_insert(
        "le",
        &le,
        sizeof(mtpl_generator),
        allocators,
        generators
    );
    return result;
}

mtpl_result mtpl_init(mtpl_context** context) {
    return mtpl_init_custom_alloc(&allocators, context);
}

mtpl_result mtpl_init_custom_alloc(
    const mtpl_allocators* allocators,
    mtpl_context** context
) {
    mtpl_result result = MTPL_SUCCESS;
    *context = allocators->malloc(sizeof(mtpl_context));
    if (!*context) {
        return MTPL_ERR_MEMORY;
    }
    (*context)->allocators = allocators;

    result = mtpl_htable_create(allocators, &((*context)->generators));
    if (result != MTPL_SUCCESS) {
        goto cleanup_context;
    }

    result = mtpl_htable_create(allocators, &((*context)->properties));
    if (result != MTPL_SUCCESS) {
        goto cleanup_generators;
    }

    result = mtpl_buffer_create(
        allocators,
        MTPL_DEFAULT_BUFSIZE,
        &((*context)->output)
    );
    if (result != MTPL_SUCCESS) {
        goto cleanup_properties;
    }

    result = add_default_generators(allocators, (*context)->generators);
    if (result != MTPL_SUCCESS) {
        goto cleanup_properties;
    }

    return MTPL_SUCCESS;

cleanup_output:
    mtpl_buffer_free(allocators, (*context)->output);
cleanup_properties:
    mtpl_htable_free(allocators, (*context)->properties);
cleanup_generators:
    mtpl_htable_free(allocators, (*context)->generators);
cleanup_context:
    allocators->free(*context);

    return result;
}

void mtpl_free(mtpl_context* context) {
    mtpl_buffer_free(context->allocators, context->output);
    mtpl_htable_free(context->allocators, context->properties);
    mtpl_htable_free(context->allocators, context->generators);
    context->allocators->free(context);
}

mtpl_result mtpl_set_generator(
    const char* name,
    mtpl_generator generator,
    mtpl_context* context
) {
    return mtpl_htable_insert(
        name,
        generator,
        sizeof(mtpl_generator), 
        context->allocators,
        context->generators
    );
}

mtpl_result mtpl_set_property(
    const char* name,
    const char* value,
    mtpl_context* context
) {
    return mtpl_htable_insert(
        name,
        value,
        strlen(value), 
        context->allocators,
        context->properties
    );
}

mtpl_result mtpl_parse_template(const char* source, mtpl_context* context) {
    context->output->cursor = 0;
    return mtpl_substitute(
        source,
        context->allocators,
        context->generators,
        context->properties,
        context->output
    );
}

