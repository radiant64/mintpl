#pragma once

#include <stddef.h>

#define MTPL_DEFAULT_BUFSIZE 1024
#define MTPL_INITIAL_DESCRIPTORS 16
#define MTPL_GENERATOR_NAME_MAXLEN 32

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
    MTPL_ERR_UNKNOWN_KEY,
    MTPL_ERR_MALFORMED_NAME
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

typedef struct {
    const char* data;
    size_t cursor;
    size_t size;
} mtpl_readbuffer;

struct mtpl_hashtable;
typedef mtpl_result(*mtpl_generator)(
    const char* arg,
    const mtpl_allocators* allocators,
    struct mtpl_hashtable* generators,
    struct mtpl_hashtable* properties,
    mtpl_buffer* out_buffer
);

