#include <mintpl/generators.h>

#include "stack.h"

#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TMP_BUF_SIZE 32

typedef enum {
    MTPL_OP_INVALID,
    MTPL_OP_ADD,
    MTPL_OP_SUBTRACT,
    MTPL_OP_MULTIPLY,
    MTPL_OP_DIVIDE,
    MTPL_OP_MODULO,
    MTPL_OP_POWER,
    MTPL_OP_NEGATE,
    MTPL_OP_LPAREN,
    MTPL_OP_RPAREN
} mtpl__operator;

static const uint8_t mtpl__precedence[] = {
    [MTPL_OP_INVALID] = 0,
    [MTPL_OP_ADD] = 2,
    [MTPL_OP_SUBTRACT] = 2,
    [MTPL_OP_MULTIPLY] = 3,
    [MTPL_OP_DIVIDE] = 4,
    [MTPL_OP_MODULO] = 4,
    [MTPL_OP_POWER] = 6,
    [MTPL_OP_NEGATE] = 5,
    [MTPL_OP_LPAREN] = 0,
    [MTPL_OP_RPAREN] = 1
};

typedef enum {
    MTPL_TOK_VALUE,
    MTPL_TOK_OP
} mtpl__token_type;

typedef struct {
    union {
        double value;
        mtpl__operator operation;
    };
    mtpl__token_type type;
} mtpl__token;

DESCRIBE_STACK(double);
DESCRIBE_STACK(mtpl__operator);
DESCRIBE_STACK(mtpl__token);

static bool mtpl__is_space(const mtpl_buffer* buf) {
    switch (buf->data[buf->cursor]) {
        case ' ': case '\t': case '\n': case '\r':
            return true;
        default: return false;
    }
}

static mtpl__operator mtpl__get_operator(
    const mtpl_buffer* buf,
    const mtpl__token* previous
) {
    const char c = buf->data[buf->cursor];
    switch (c) {
    case '+': return MTPL_OP_ADD;
    case '-': 
        // If the previous token was an operator, this must be a unary minus,
        // except if the "operator" was a right paren.
        if (
            previous->type == MTPL_TOK_OP
            && previous->operation != MTPL_OP_RPAREN
        ) {
            return MTPL_OP_NEGATE;
        }
        return MTPL_OP_SUBTRACT;
    case '*': return MTPL_OP_MULTIPLY;
    case '/': return MTPL_OP_DIVIDE;
    case '%': return MTPL_OP_MODULO;
    case '^': return MTPL_OP_POWER;
    case '(': return MTPL_OP_LPAREN;
    case ')': return MTPL_OP_RPAREN;

    default: return MTPL_OP_INVALID;
    }
}

static mtpl_result mtpl__extract_number(mtpl_buffer* buf, double* out) {
    char* end = NULL;
    errno = 0;
    *out = strtod(&buf->data[buf->cursor], &end);
    if (errno != 0 || end == &buf->data[buf->cursor]) {
        return MTPL_ERR_SYNTAX;
    }

    buf->cursor += (uintptr_t) end - (uintptr_t) &buf->data[buf->cursor];

    return MTPL_SUCCESS;
}

static mtpl_result mtpl__dump_ops(
    STACK(mtpl__token)* expr,
    STACK(mtpl__operator)* ops,
    mtpl__operator op
) {
    mtpl__token token = { 0, MTPL_TOK_OP };
    mtpl__operator* top;
    while (
        (top = PEEK_BACK(mtpl__operator)(ops))
        && mtpl__precedence[op] <= mtpl__precedence[*top]
    ) {
        token.operation = *top;
        const mtpl_result res = PUSH_BACK(mtpl__token)(expr, &token);
        if (res != MTPL_SUCCESS) {
            return res;
        }
        ops->num_entries--;
    }
    if (op == MTPL_OP_INVALID) {
        return MTPL_SUCCESS;
    } else if (op == MTPL_OP_RPAREN) {
        return (top && *top == MTPL_OP_LPAREN) ? MTPL_SUCCESS : MTPL_ERR_SYNTAX;
    }
    return PUSH_BACK(mtpl__operator)(ops, &op); 
}

static mtpl_result mtpl__parse_expr(
    const mtpl_allocators* allocators,
    mtpl_buffer* buf,
    STACK(mtpl__token)** out_expr
) {
    mtpl_result res = MTPL_SUCCESS;
    STACK(mtpl__operator)* ops = CREATE_STACK(mtpl__operator)(allocators);
    if (!ops) {
        return MTPL_ERR_MEMORY;
    }
    STACK(mtpl__token)* expr = CREATE_STACK(mtpl__token)(allocators);
    if (!expr) {
        res = MTPL_ERR_MEMORY;
        goto cleanup_ops;
    }

    // Special state: The beginning of the expression is an "invisible" token
    // describing an invalid operation, to be able to correctly parse a unary
    // minus at the start of an expression.
    mtpl__token token = { .operation = MTPL_OP_INVALID, true };
    while (buf->cursor < buf->size) {
        if (mtpl__is_space(buf)) {
            buf->cursor++;
            continue;
        }

        const mtpl__operator op = mtpl__get_operator(buf, &token);
        token.type = (op != MTPL_OP_INVALID) ? MTPL_TOK_OP : MTPL_TOK_VALUE;
        switch (token.type) {
        case MTPL_TOK_VALUE:
            res = mtpl__extract_number(buf, &token.value);
            if (res == MTPL_SUCCESS) {
                res = PUSH_BACK(mtpl__token)(expr, &token);
            }
            break;
        case MTPL_TOK_OP:
            res = mtpl__dump_ops(expr, ops, op);
            if (res != MTPL_SUCCESS) {
                goto err_cleanup_expr;
            }
            buf->cursor++;
            break;
        }

        if (res != MTPL_SUCCESS) {
            goto err_cleanup_expr;
        }
    }

    res = mtpl__dump_ops(expr, ops, MTPL_OP_INVALID);
    if (res != MTPL_SUCCESS) {
        goto err_cleanup_expr;
    }

    *out_expr = expr;

cleanup_ops:
    FREE_STACK(mtpl__operator)(ops);
    return res;

err_cleanup_expr:
    FREE_STACK(mtpl__token)(expr);
    goto cleanup_ops;
}

static mtpl_result mtpl__eval_valstack(
    STACK(double)* stack,
    mtpl__operator operation
) {
    double a;
    double b;
    switch (operation) {
    case MTPL_OP_ADD:
    case MTPL_OP_SUBTRACT:
    case MTPL_OP_MULTIPLY:
    case MTPL_OP_DIVIDE:
    case MTPL_OP_MODULO:
    case MTPL_OP_POWER:
        if (stack->num_entries != 2) {
            return MTPL_ERR_SYNTAX;
        }
        b = *POP_BACK(double)(stack);
        a = *POP_BACK(double)(stack);
        break;
    case MTPL_OP_NEGATE:
        if (stack->num_entries != 1) {
            return MTPL_ERR_SYNTAX;
        }
        a = *POP_BACK(double)(stack);
        break;
    default:
        return MTPL_ERR_SYNTAX;
    }

    double value;
    switch (operation) {
    case MTPL_OP_ADD:
        value = a + b;
        break;
    case MTPL_OP_SUBTRACT:
        value = a - b;
        break;
    case MTPL_OP_MULTIPLY:
        value = a * b;
        break;
    case MTPL_OP_DIVIDE:
        value = a / b;
        break;
    case MTPL_OP_MODULO:
        value = fmod(a, b);
        break;
    case MTPL_OP_POWER:
        value = pow(a, b);
        break;
    case MTPL_OP_NEGATE:
        value = -a;
        break;
    default:
        return MTPL_ERR_SYNTAX;
    }
    return PUSH_BACK(double)(stack, &value);
}

static mtpl_result mtpl__eval_expr(
    const mtpl_allocators* allocators,
    const STACK(mtpl__token)* expr,
    mtpl_buffer* out
) {
    char tmp_data[TMP_BUF_SIZE];
    mtpl_result res = MTPL_SUCCESS;
    STACK(double)* valstack = CREATE_STACK(double)(allocators);
    if (!valstack) {
        return MTPL_ERR_MEMORY;
    }

    const mtpl__token* cursor = &expr->entries[0];
    for (size_t i = 0; i < expr->num_entries; ++i, cursor = &expr->entries[i]) {
        switch (cursor->type) {
        case MTPL_TOK_VALUE:
            PUSH_BACK(double)(valstack, &cursor->value);
            break;
        case MTPL_TOK_OP:
            res = mtpl__eval_valstack(valstack, cursor->operation);
            if (res != MTPL_SUCCESS) {
                goto cleanup_valstack;
            }
            break;
        }
    }

    if (valstack->num_entries != 1) {
        res = MTPL_ERR_SYNTAX;
        goto cleanup_valstack;
    }

    const size_t len = snprintf(tmp_data, 0, "%g", valstack->entries[0]) + 1;
    if (len > TMP_BUF_SIZE) {
        res = MTPL_ERR_MALFORMED_NAME;
        goto cleanup_valstack;
    }
    mtpl_buffer tmp_buf = { tmp_data };
    snprintf(tmp_data, len, "%g", valstack->entries[0]);
    res = mtpl_buffer_print(&tmp_buf, allocators, out);

cleanup_valstack:
    FREE_STACK(double)(valstack);

    return res;
}

mtpl_result mtpl_generator_arithmetics(
    const mtpl_allocators* allocators,
    mtpl_buffer* arg,
    mtpl_hashtable* generators,
    mtpl_hashtable* properties,
    mtpl_buffer* out
) {
    STACK(mtpl__token)* expr;
    mtpl_result res = mtpl__parse_expr(allocators, arg, &expr);
    if (res == MTPL_SUCCESS) {
        res = mtpl__eval_expr(allocators, expr, out);
        FREE_STACK(mtpl__token)(expr);
    }
    
    return res;
}

