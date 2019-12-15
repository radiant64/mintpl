#pragma once

#include <stddef.h>

#define MTPL_DEFAULT_BUFSIZE 1024
#define MTPL_INITIAL_DESCRIPTORS 16

#define MTPL_REALLOC_CHECKED(allocators, addr, size)\
    do {\
        void* res_addr = allocators->realloc(addr, size);\
        if (!res_addr) {\
            return MTPL_ERR_MEMORY;\
        }\
        addr = res_addr;\
    } while(0)

typedef enum {
    MTPL_SUCCESS,
    MTPL_ERR_MEMORY,
    MTPL_ERR_SYNTAX,
    MTPL_ERR_UNKNOWN_GENERATOR
} mtpl_result;

typedef struct {
    void* (*malloc)(size_t);
    void* (*realloc)(void*, size_t);
    void (*free)(void*);
} mtpl_allocators;

typedef struct {
    char* data;
    size_t cursor;
    size_t size;
} mtpl_buffer;

struct mtpl_descriptors;
typedef mtpl_result(*mtpl_generator)(
    const char* arg,
    const mtpl_allocators* allocators,
    struct mtpl_descriptors* descriptors,
    mtpl_buffer* out_buffer
);

typedef struct mtpl_descriptors {
    char** names;
    mtpl_generator* generators;
    size_t count;
    size_t capacity;
} mtpl_descriptors;

