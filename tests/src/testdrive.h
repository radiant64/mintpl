#pragma once
/*
Copyright (c) 2021 Martin Evald

MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED *AS IS*, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <assert.h>
#include <setjmp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TD_VERSION 1.0.0

#ifndef TD_MAX_SECTIONS
#define TD_MAX_SECTIONS 128
#endif
#ifndef TD_MAX_NESTING
#define TD_MAX_NESTING 32
#endif
#ifndef TD_MAX_ASSERTS
#define TD_MAX_ASSERTS 256
#endif

#define TD_TEST_INFO(NAME) td__test_ ## NAME
#define TD_TEST_FUNCTION(NAME) td__test_fn_ ## NAME

#define TD_ALLOC_SECTIONS(CONTEXT)\
    if (!CONTEXT->sections) {\
        CONTEXT->sections = malloc(\
            sizeof(struct td_test_context) * TD_MAX_SECTIONS\
        );\
    }

#define TD_EVENT(EVENT_, TEST_)\
    if (!td__retracing) {\
        td_listener(EVENT_, TEST_, td__path[td__level], __FILE__, __LINE__);\
    }

// Note: In order to be able to assert using the REQUIRE() macro, and proceed
// to run the next test section on failure, a combination of setjmp() and
// longjmp() are employed. A failing assertion will longjmp() back to the
// location referenced by the td__continue pointer, from where the conditional
// executed when returning from setjmp() will advance to the next sequence (if
// there were any unexecuted sections preceeding the failing assertion), or
// break out from the current scope.
#define TD_BREAK_HERE(CONTINUE_)\
    if (setjmp(CONTINUE_) == 1) {\
        if (++td__path[td__level] > td__test_ptr->section_idx) {\
            break;\
        }\
    }

#define TD_FIXTURE(NAME, DESCRIPTION)\
    struct td_test_context TD_TEST_INFO(NAME) = {\
        NULL,\
        #NAME,\
        DESCRIPTION\
    };\
    bool TD_TEST_FUNCTION(NAME)(struct td_test_context* td__test_ptr) {\
        TD_ALLOC_SECTIONS(td__test_ptr);\
        size_t td__path[TD_MAX_NESTING] = { 0 };\
        size_t td__visited[TD_MAX_NESTING] = { 0 };\
        size_t td__level = 0;\
        struct td_test_context* td__root = td__test_ptr;\
        bool td__retracing = false;\
        size_t td__target_level = 0;\
        size_t td__assert_count = 0;\
        TD_EVENT(TD_TEST_START, td__test_ptr);\
        jmp_buf td__begin;\
        jmp_buf* td__continue = &td__begin;\
        do {\
            TD_BREAK_HERE(td__begin)\
            td__test_ptr->section_idx = 0;\

#define TD_SECTION(DESCRIPTION)\
    TD_EVENT(TD_SECTION_PRE, td__test_ptr);\
    assert(td__test_ptr->section_idx < TD_MAX_SECTIONS);\
    if (\
        td__test_ptr->sections[++(td__test_ptr->section_idx)].name\
            == NULL\
    ) {\
        td__test_ptr->sections[td__test_ptr->section_idx]\
            = (struct td_test_context) {\
                td__test_ptr,\
                "",\
                DESCRIPTION\
            };\
    }\
    if (\
        td__path[td__level] != td__test_ptr->section_idx) {\
        TD_EVENT(\
            TD_SECTION_SKIP,\
            &td__test_ptr->sections[td__test_ptr->section_idx]\
        );\
    } else {\
        {\
            if (!td__retracing) {\
                td__retracing = true;\
                td__target_level = td__level + 1;\
                td__level = 0;\
                td__test_ptr = td__root;\
                longjmp(td__begin, 2);\
            }\
            jmp_buf* td__parent_continue = td__continue;\
            td__test_ptr = &td__test_ptr->sections[td__path[td__level++]];\
            TD_ALLOC_SECTIONS(td__test_ptr);\
            if (td__retracing && td__level == td__target_level) {\
                td__retracing = false;\
            }\
            TD_EVENT(TD_SECTION_START, td__test_ptr);\
            jmp_buf td__section_begin;\
            td__continue = &td__section_begin;\
            do {\
                TD_BREAK_HERE(td__section_begin)\
                td__test_ptr->section_idx = 0;\

#define TD_END_SECTION\
            } while (td__path[td__level]++ < td__test_ptr->section_idx);\
            TD_EVENT(TD_SECTION_END, td__test_ptr);\
            td__test_ptr = td__test_ptr->parent;\
            td__continue = td__parent_continue;\
            td__path[td__level] = 0;\
            td__level--;\
        }\
    }

#define TD_END_FIXTURE\
        } while (td__test_ptr->section_idx > td__path[td__level]++);\
        TD_EVENT(TD_TEST_END, td__test_ptr);\
        {\
            bool success = true;\
            for (size_t i = 0; i < td__test_ptr->assert_count; ++i) {\
                success &= td__test_ptr->assert_success[i];\
            }\
            return success ? 0 : 1;\
        }\
    }

#define TD_EXTERN_FIXTURE(NAME)\
    extern struct td_test_context TD_TEST_INFO(NAME);\
    void TD_TEST_FUNCTION(NAME)(struct td_test*, const size_t)

#define TD_ASSERT_LOGIC(CONDITION, SUCCESS, FAILURE)\
    if (td__path[td__level] == 0) {\
        td__test_ptr->assertions[td__test_ptr->assert_count++] = #CONDITION;\
        TD_EVENT(TD_ASSERT_PRE, td__test_ptr);\
        if (!(CONDITION)) {\
            FAILURE;\
        } else {\
            SUCCESS;\
        }\
    }

#define TD_REQUIRE(CONDITION) TD_ASSERT_LOGIC(\
        CONDITION, ({\
            td__test_ptr->assert_success[td__test_ptr->assert_count - 1]\
                = true;\
            TD_EVENT(TD_ASSERT_SUCCESS, td__test_ptr)\
        }), ({\
            td__test_ptr->assert_success[td__test_ptr->assert_count - 1]\
                = false;\
            TD_EVENT(TD_ASSERT_FAILURE, td__test_ptr);\
            longjmp(*td__continue, 1);\
        })\
    )

#define TD_REQUIRE_FAIL(CONDITION) TD_ASSERT_LOGIC(\
        CONDITION, ({\
            td__test_ptr->assert_success[td__test_ptr->assert_count - 1]\
                = false;\
            TD_EVENT(TD_ASSERT_FAILURE, td__test_ptr);\
        }), ({\
            td__test_ptr->assert_success[td__test_ptr->assert_count - 1]\
                = true;\
            TD_EVENT(TD_ASSERT_SUCCESS, td__test_ptr);\
        })\
    );\
    longjmp(*td__continue, 1);


#define TD_SET_LISTENER(LISTENER_FUNC) td_listener = LISTENER_FUNC

#define TD_RUN_TEST(NAME) TD_TEST_FUNCTION(NAME)(&TD_TEST_INFO(NAME))

#ifndef TD_DEFAULT_LISTENER
#define TD_DEFAULT_LISTENER td_console_listener
#endif

#ifndef TD_ONLY_PREFIXED_MACROS

#define FIXTURE(...) TD_FIXTURE(__VA_ARGS__)
#define SECTION(...) TD_SECTION(__VA_ARGS__)
#define END_SECTION TD_END_SECTION
#define END_FIXTURE TD_END_FIXTURE
#define EXTERN_FIXTURE(...) TD_EXTERN_FIXTURE(__VA_ARGS__)

#define REQUIRE(...) TD_REQUIRE(__VA_ARGS__)
#define REQUIRE_FAIL(...) TD_REQUIRE_FAIL(__VA_ARGS__)

#define RUN_TEST(...) TD_RUN_TEST(__VA_ARGS__)

#endif

enum td_event {
    TD_TEST_START,
    TD_SECTION_PRE,
    TD_SECTION_SKIP,
    TD_SECTION_START,
    TD_SECTION_END,
    TD_ASSERT_PRE,
    TD_ASSERT_FAILURE,
    TD_ASSERT_SUCCESS,
    TD_TEST_END
};

struct td_test_context {
    struct td_test_context* parent;
    const char* name;
    const char* description;
    size_t section_idx;
    size_t assert_count;
    bool assert_success[TD_MAX_ASSERTS];
    const char* assertions[TD_MAX_ASSERTS];
    struct td_test_context* sections;
};

static void td_console_listener(
    enum td_event event,
    struct td_test_context* test,
    size_t sequence,
    const char* file,
    size_t line
);

static void(*td_listener)(
    enum td_event event,
    struct td_test_context* test,
    size_t sequence,
    const char* file,
    size_t line
) = TD_DEFAULT_LISTENER;

// Default console listener implementation below this line.

static const char* td__indent(size_t indent) {
    static char buffer[256] = { 0 };
    for (size_t i = 0; i < indent; ++i) {
        memcpy(&buffer[i * 3], "|  ", 3);
    }
    buffer[indent * 3] = '\0';
    return buffer;
}

static void td__print_ratio(
    size_t indent,
    size_t successful,
    size_t total
) {
    fprintf(
        stdout,
        "%s+- %ld/%ld assertions succeded.\n",
        td__indent(indent),
        successful,
        total
    );
}

static size_t td__count_success(struct td_test_context* test) {
    size_t count = 0;
    for (size_t i = 0; i < test->assert_count; ++i) {
        count += test->assert_success[i];
    }
    return count;
}

static void td_console_listener(
    enum td_event event,
    struct td_test_context* test,
    size_t sequence,
    const char* file,
    size_t line
) {
    static size_t indent = 0;
    switch (event) {
    case TD_TEST_START:
        fprintf(stdout, "Running test: \"%s\"\n", test->description);
        indent++;
        break;
    case TD_SECTION_PRE:
        break;
    case TD_SECTION_SKIP:
        break;
    case TD_SECTION_START:
        fprintf(
            stdout,
            "%sRunning section: \"%s\"\n",
            td__indent(indent),
            test->description
        );
        indent++;
        break;
    case TD_SECTION_END:
        indent--;
        td__print_ratio(indent, td__count_success(test), test->assert_count);
        break;
    case TD_ASSERT_PRE:
        break;
    case TD_ASSERT_FAILURE:
        fprintf(
            stdout,
            "%sFailed assertion: %s (%s:%ld)\n",
            td__indent(indent),
            test->assertions[test->assert_count - 1],
            file,
            line
        );
        break;
    case TD_ASSERT_SUCCESS:
        break;
    case TD_TEST_END:
        indent--;
        td__print_ratio(indent, td__count_success(test), test->assert_count);
        break;
    }
}

