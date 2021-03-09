#ifndef _CONTEXT_FASTEST
#define _CONTEXT_FASTEST 1

#ifdef _FORTIFY_SOURCE
#    undef _FORTIFY_SOURCE
#endif

#ifndef _XOPEN_SOURCE
#    define _XOPEN_SOURCE 999
#endif
#include <ucontext.h>
#include <setjmp.h>
#include <stdint.h>

typedef struct context_fastest_s {
    uint64_t sp;
    uint64_t ra;
    uint64_t s0;
    uint64_t a0;
    uint64_t a1;
    uint64_t a2;
    uint64_t a3;
    uint64_t a4;
    uint64_t a5;
    uint64_t a6;
    uint64_t a7;
    double ft0;
    double ft1;
    double ft2;
    double ft3;
    double ft4;
    double ft5;
    double ft6;
    double ft7;
    double ft8;
    double ft9;
    double ft10;
    double ft11;
    double fa0;
    double fa1;
    double fa2;
    double fa3;
    double fa4;
    double fa5;
    double fa6;
    double fa7;
} context_fastest_t[1];

void initialize_context_fastest(context_fastest_t ctx,
                                void *stack_base,
                                size_t stack_size,
                                void (*entry)(void *data),
                                void *data);

void switch_context_fastest(context_fastest_t from,
                            context_fastest_t to);

#endif
