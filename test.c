#define VERSION 2

#if VERSION == 0
// 225.3 ns on i7-6700HQ CPU @ 2.6 GHz running Ubuntu 16.04.01
#include "context_posix.h"
#define context_t          context_posix_t
#define switch_to          switch_context_posix
#define initialize_context initialize_context_posix
#endif

#if VERSION == 1
// 21.1 ns on i7-6700HQ CPU @ 2.6 GHz running Ubuntu 16.04.01
#include "context_fast.h"
#define context_t          context_fast_t
#define switch_to          switch_context_fast
#define initialize_context initialize_context_fast
#endif

#if VERSION == 2
// 0.3 ns on Zen 2 3900XT on Ubuntu 21.04
#include "context_fastest.h"
#define context_t          context_fastest_t
#define initialize_context initialize_context_fastest

#ifdef __amd64
/*
 * My gcc asm-fu isn't strong enough to figure out how to eliminate
 * the two dummy instructions before the call (they turn into movq
 * %rcx,%rcx and movq %rdx,%rdx respectively, but you can't know that
 * in general).
 *
 * Kudos to Jose Renau for suggesting the use of asm clobbers.
 */
#define switch_to(from, to)                                             \
    __asm__("movq %0,%%rcx\n"                                           \
            "movq %1,%%rdx\n"                                           \
            "callq *0(%%rdx)\n"                                         \
            "popq  0(%%rcx)\n"                                          \
            "movq  %%rsp,8(%%rcx)\n"                                    \
            "movq  8(%%rdx),%%rsp\n"                                    \
            :                                                           \
            : "r" (from), "r" (to)                                      \
            : "cc", "memory", "%rbx", "%rbp", "%r12", "%r13", "%r14", "%r15");

#define initialize_context initialize_context_fastest
#endif
#endif

#ifdef __riscv

// Use asm() directives to trick GCC into spilling only caller-save
// registers that may be live at this point.

#define switch_to(from, to)                                             \
    __asm__("la      a4,%0\n"                                           \
            "la      a5,%1\n"                                           \
            "ld      t0,(a5)\n"                                         \
            "jalr    t0\n"                                              \
            "sd      ra,(a5)\n"                                         \
            "sd      sp,8(a4)\n"                                        \
            "ld      sp,8(a5)\n"                                        \
            :                                                           \
            : "r" (from), "r" (to)                                      \
            : "cc", "memory",                                           \
              "ra", "s0",                                               \
              "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",           \
              "ft0", "ft1", "ft2", "ft3", "ft4", "ft5", "ft6", "ft7",   \
              "ft8", "ft9", "ft10", "ft11",                             \
              "fa0", "fa1", "fa2", "fa3", "fa4", "fa5", "fa6", "fa7");
#endif


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

#define DEBUG(fmt...)
//#define DEBUG(fmt...) printf(fmt)

#define ITERATIONS 10000000
//#define ITERATIONS 1
//#define ITERATIONS 0

#define STACKSIZE 32768

context_t thread1, thread2, thread3;

void thread1_fun(void *data)
{
    int i;

    struct timeval tv0, tv;

    gettimeofday(&tv0, NULL);

    for (i = 0; i < ITERATIONS; ++i) {
        DEBUG("thread1 (%p): %d\n", data, i);
        switch_to(thread1, thread2);
    }

    gettimeofday(&tv, NULL);

    double t = tv.tv_sec - tv0.tv_sec + 1e-6 * (tv.tv_usec - tv0.tv_usec);
    double f = 3 * ITERATIONS / t;

    if (ITERATIONS)
        printf("%d context switches in %.1f s, %.1f/s, %.1f ns each\n",
               3 * ITERATIONS, t, f, 1e9/f);

    printf("done\n");
}

void thread2_fun(void *data)
{
    int i;

    for (i = 0; i < ITERATIONS; ++i) {
        DEBUG("thread2 (%p): %d\n", data, i);
        switch_to(thread2, thread3);
    }
}

void thread3_fun(void *data)
{
    int i;

    for (i = 0; i < ITERATIONS; ++i) {
        DEBUG("thread3 (%p): %d\n", data, i);
        switch_to(thread3, thread1);
    }
}

// cache line
#define ALIGN 64

void *aligned_malloc(int size) {
    void *mem = malloc(size+ALIGN+sizeof(void*));
    void **ptr = (void**)((uintptr_t)(mem+ALIGN+sizeof(void*)) & ~(ALIGN-1));
    ptr[-1] = mem;
    return ptr;
}

void aligned_free(void *ptr) {
    free(((void**)ptr)[-1]);
}

#define RED_ZONE 128

static void init_context(context_t ctx, void (*entry)(void *data), void *data)
{
    // Ok, a fair amount of AMD64 ABI cruft snuck in here.
    // See fx. https://software.intel.com/en-us/forums/intel-isa-extensions/topic/291241
    void *stack = malloc(STACKSIZE + RED_ZONE + 16);
    DEBUG("%p: stack = [%p; %p)\n",
          data, stack, stack + STACKSIZE + RED_ZONE + 16);
    initialize_context(ctx, stack, STACKSIZE, entry, data);
}

int main()
{
    init_context(thread2, thread2_fun, (void *) 0xDEEEECAF);
    init_context(thread3, thread3_fun, (void *) 0xF000000D);
    thread1_fun((void *) 0xBABEBABE);

    return 0;
}
