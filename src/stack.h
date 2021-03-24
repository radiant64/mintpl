#pragma once

#define INITIAL_STACK 16

#define SYM(A, B) A ## B

#define STACK_SYM(TYPE) mtpl__ ## TYPE ## _stack
#define STACK(TYPE) STACK_SYM(TYPE)
#define CREATE_STACK_SYM(TYPE) SYM(TYPE, _create)
#define FREE_STACK_SYM(TYPE) SYM(TYPE, _free)
#define PUSH_BACK_SYM(TYPE) SYM(TYPE, _push_back)
#define POP_BACK_SYM(TYPE) SYM(TYPE, _pop_back)
#define PEEK_BACK_SYM(TYPE) SYM(TYPE, _peek_back)

#define CREATE_STACK(TYPE) CREATE_STACK_SYM(STACK(TYPE))
#define FREE_STACK(TYPE) FREE_STACK_SYM(STACK(TYPE))
#define PUSH_BACK(TYPE) PUSH_BACK_SYM(STACK(TYPE))
#define POP_BACK(TYPE) POP_BACK_SYM(STACK(TYPE))
#define PEEK_BACK(TYPE) PEEK_BACK_SYM(STACK(TYPE))

#define DEFINE_CREATE_STACK(TYPE)\
    static STACK(TYPE)* CREATE_STACK(TYPE)(\
        const mtpl_allocators* allocators\
    ) {\
        STACK(TYPE)* stack = allocators->malloc(sizeof(STACK(TYPE)));\
        if (!stack) {\
            return NULL;\
        }\
        stack->entries = allocators->malloc(sizeof(TYPE) * INITIAL_STACK);\
        if (!stack->entries) {\
            allocators->free(stack);\
            return NULL;\
        }\
        stack->allocators = allocators;\
        stack->num_entries = 0;\
        stack->cap_entries = INITIAL_STACK;\
        return stack;\
    }

#define DEFINE_FREE_STACK(TYPE)\
    static void FREE_STACK(TYPE)(STACK(TYPE)* stack) {\
        stack->allocators->free(stack->entries);\
        stack->allocators->free(stack);\
    }

#define DEFINE_PUSH_BACK(TYPE)\
    static mtpl_result PUSH_BACK(TYPE)(STACK(TYPE)* stack, const TYPE* value) {\
        if (stack->num_entries == stack->cap_entries) {\
            MTPL_REALLOC_CHECKED(\
                stack->allocators,\
                stack->entries,\
                sizeof(TYPE) * stack->cap_entries * 2,\
                return MTPL_ERR_MEMORY\
            );\
            stack->cap_entries *= 2;\
        }\
        stack->entries[stack->num_entries++] = *value;\
        return MTPL_SUCCESS;\
    }

#define DEFINE_POP_BACK(TYPE)\
    static TYPE* POP_BACK(TYPE)(STACK(TYPE)* stack) {\
        if (!stack->num_entries) {\
            return NULL;\
        }\
        return &stack->entries[--(stack->num_entries)];\
    }

#define DEFINE_PEEK_BACK(TYPE)\
    static TYPE* PEEK_BACK(TYPE)(STACK(TYPE)* stack) {\
        if (!stack->num_entries) {\
            return NULL;\
        }\
        return &stack->entries[stack->num_entries - 1];\
    }

#define DESCRIBE_STACK(TYPE)\
    typedef struct {\
        const mtpl_allocators* allocators;\
        size_t num_entries;\
        size_t cap_entries;\
        TYPE* entries;\
    } STACK(TYPE);\
    DEFINE_CREATE_STACK(TYPE);\
    DEFINE_FREE_STACK(TYPE);\
    DEFINE_PUSH_BACK(TYPE);\
    DEFINE_POP_BACK(TYPE);\
    DEFINE_PEEK_BACK(TYPE)

