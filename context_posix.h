#ifndef _CONTEXT_POSIX
#define _CONTEXT_POSIX 1

#ifdef _FORTIFY_SOURCE
#    undef _FORTIFY_SOURCE
#endif

#ifndef _XOPEN_SOURCE
#    define _XOPEN_SOURCE 999
#endif
#include <ucontext.h>
#include <setjmp.h>


typedef struct context_posix_s {
    ucontext_t  ucontext;
    jmp_buf     jmp;
} context_posix_t[1];

void initialize_context_posix(context_posix_t ctx,
                              void *stack_base,
                              size_t stack_size,
                              void (*entry)(void *data),
                              void *data);

void switch_context_posix(context_posix_t from,
                          context_posix_t to);

#endif
