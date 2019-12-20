#pragma once

#include <stddef.h>

#ifdef _cplusplus
extern "C" {
#endif

#define MTPL_DEFAULT_BUFSIZE 1024
#define MTPL_INITIAL_DESCRIPTORS 16
#define MTPL_GENERATOR_NAME_MAXLEN 32

#define MTPL_REALLOC_CHECKED(allocators, addr, size, errcon)\
    do {\
        void* res_addr = allocators->realloc(addr, size);\
        if (!res_addr) {\
            errcon;\
        }\
        addr = res_addr;\
    } while(0)

typedef enum {
    MTPL_SUCCESS,
    MTPL_ERR_MEMORY,
    MTPL_ERR_SYNTAX,
    MTPL_ERR_UNKNOWN_KEY,
    MTPL_ERR_MALFORMED_NAME
} mtpl_result;

typedef struct {
    void* (*malloc)(size_t);
    void* (*realloc)(void*, size_t);
    void (*free)(void*);
} mtpl_allocators;

#ifdef _cplusplus
}
#endif

