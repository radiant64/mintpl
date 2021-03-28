#include "locale.h"

#include <mintpl/mintpl.h>
#include <mintpl/buffers.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define VERSION "1.0.0"

typedef struct {
    FILE* in;
    FILE* out;
    mtpl_context* ctx;
} invocation_data;

void display_usage(const char* name) {
    fprintf(stdout, l_usage, name);
}

int process_invocation(int argc, char** argv, invocation_data* run) {
    int opt;

    run->in = NULL;
    run->out = NULL;

    mtpl_result result = mtpl_init(&(run->ctx));
    if (result != MTPL_SUCCESS) {
        fprintf(stderr, l_err_mtpl_init_failed, result);
        return result;
    }

    int i = 0;
    char* value;
    while ((opt = getopt(argc, argv, "ho:p:v?")) != -1) {
        switch (opt) {
        case 'h':
        case '?':
            display_usage(argv[0]);
            return 0;
        case 'o':
            run->out = fopen(optarg, "w");
            if (!run->out) {
                fprintf(stderr, l_err_open_out_failed, optarg);
                return 2;
            }
            break;
        case 'p':
            i = 0;
            while (optarg[++i] && optarg[i] != '=') {
            };
            if (!optarg[i]) {
                fprintf(stderr, l_err_malformed_prop, optarg);
                display_usage(argv[0]);
                return 3;
            }
            optarg[i] = '\0';
            value = &(optarg[i + 1]);
            result = mtpl_set_property(optarg, value, run->ctx);
            if (result != MTPL_SUCCESS) {
                fprintf(stderr, l_err_set_prop, result);
                return 7;
            }
            break;
        case 'v':
            fprintf(stdout, l_version, VERSION, mtpl_version());
            return 0;
        default:
            fprintf(stderr, l_err_unknown_opt, opt);
            display_usage(argv[0]);
            return 1;
        }
    }

    run->in = optind < argc ? fopen(argv[optind], "r") : stdin;

    if (!run->in) {
        fprintf(
            stderr,
            l_err_open_in_failed,
            optind < argc ? argv[optind] : "stdin"
        );
        return 2;
    }
    if (!run->out) {
        run->out = stdout;
    }
    
    return 0;
}

int main(int argc, char** argv) {
    invocation_data run;

    int result = process_invocation(argc, argv, &run);
    if (result != 0 || !run.in || !run.out) {
        return result; 
    }

    char indata[1025];
    mtpl_buffer in = { indata };
    mtpl_buffer template = {
        .data = malloc(1024),
        .size = 1024
    };
    size_t read_bytes = 0;
    while (read_bytes = fread(indata, 1, 1024, run.in)) {
        indata[read_bytes] = '\0';
        in.cursor = 0;
        mtpl_buffer_print(&in, run.ctx->allocators, &template);
    };
    fclose(run.in);
    result = mtpl_parse_template(template.data, run.ctx);
    free(template.data);
    if (result != MTPL_SUCCESS) {
        fprintf(stderr, l_err_parse, result, 0);
        exit(5);
    }

    size_t wrote_bytes = fwrite(
        run.ctx->output->data,
        1,
        run.ctx->output->cursor,
        run.out
    );
    if (wrote_bytes != run.ctx->output->cursor) {
        fprintf(stderr, l_err_write, run.ctx->output->cursor);
        exit(6);
    }

    return 0;
}

