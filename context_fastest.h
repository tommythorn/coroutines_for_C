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


typedef struct context_fastest_s {
    unsigned long *sp;
} context_fastest_t[1];

void initialize_context_fastest(context_fastest_t ctx,
                                void *stack_base,
                                size_t stack_size,
                                void (*entry)(void *data),
                                void *data);

void switch_context_fastest(context_fastest_t from,
                            context_fastest_t to);

#endif
