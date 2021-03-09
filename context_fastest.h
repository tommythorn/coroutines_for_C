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
#ifdef __amd64
    unsigned long *sp;
#endif

#ifdef __riscv
    uint64_t ra;
    uint64_t sp;
    uint64_t s0;
    uint64_t s1;
    uint64_t s2;
    uint64_t s3;
    uint64_t s4;
    uint64_t s5;
    uint64_t s6;
    uint64_t s7;
    uint64_t s8;
    uint64_t s9;
    uint64_t s10;
    uint64_t s11;
    double   fs0;
    double   fs1;
    double   fs2;
    double   fs3;
    double   fs4;
    double   fs5;
    double   fs6;
    double   fs7;
    double   fs8;
    double   fs9;
    double   fs10;
    double   fs11;

    void (*entry)(void *); // @ 208
    void *data;
#endif
} context_fastest_t[1];

void initialize_context_fastest(context_fastest_t ctx,
                                void *stack_base,
                                size_t stack_size,
                                void (*entry)(void *data),
                                void *data);

void switch_context_fastest(context_fastest_t from,
                            context_fastest_t to);

#endif
