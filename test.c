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
#define switch_context(from, to)                                        \
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
/* switch_context is at the heart of the coroutines.  We use asm()
   directives to trick GCC into spilling only callee-save registers
   that may be live at this point, thus here we only have to do the
   following things:

   1. Save our sp and resume point in the "from" context.
   2. Load our new sp and resumption point from the "to" context.
   3. Jump to it.

   Here we take advantage of the jalr function to set the return
   address in ra, which means that the save of it ends up in the
   receiver (this is confusing, but single stepping through it might
   clear things up).

   Because of this and because of the helper function to get things
   going, it's required that we fix the registers that pass from and
   to.  Unfortunately, I don't know of a way to force %0 and %1 to be
   allocated to two fixed registers, so I can't get rid of the two
   moves at the beginning.

   Final issue: the jalr below doesn't follow the convention that will
   signal a coroutine jump.  This SHOULD be fixed, but it's slightly
   expensive as it requires using t0 which isn't a callee save and
   thus we would have to save and restore it also.
*/

#define switch_context(from, to)                                        \
  __asm__("addi    s0,%0,0\n"                                           \
          "addi    s1,%1,0\n"                                           \
          "ld      s2,(s1)\n"                                           \
          "sd      sp,8(s0)\n"                                          \
          "ld      sp,8(s1)\n"                                          \
          "jalr    s2\n"                                                \
          "sd      ra,(s0)\n"                                           \
          :                                                             \
          : "r" (from), "r" (to)                                        \
          : "memory",                                                   \
            "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9", \
            "s10", "s11", "fs0", "fs1", "fs2", "fs3", "fs4", "fs5",     \
            "fs6", "fs7", "fs8", "fs9", "fs10", "fs11")
#endif


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

//#define DEBUG(fmt...)
#define DEBUG(fmt...) printf(fmt)

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
        switch_context(thread1, thread2);
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
    DEBUG("thread2 (%p): -> 1\n", data);
    switch_context(thread2, thread1);

    DEBUG("thread2 (%p): -> 1\n", data);
    switch_context(thread2, thread1);

    DEBUG("thread2 (%p): -> 3\n", data);
    switch_context(thread2, thread3);

    DEBUG("thread2 (%p): -> 1\n", data);
    switch_context(thread2, thread1);

    for (int i = 0; i < ITERATIONS; ++i) {
        DEBUG("thread2 (%p): %d\n", data, i);
        switch_context(thread2, thread3);
    }
}

void thread3_fun(void *data)
{
    DEBUG("thread3 (%p): -> 2\n", data);
    switch_context(thread3, thread2);

    DEBUG("thread3 (%p): -> 1\n", data);
    switch_context(thread3, thread1);

    for (int i = 0; i < ITERATIONS; ++i) {
        DEBUG("thread3 (%p): %d\n", data, i);
        switch_context(thread3, thread1);
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
