#define VERSION 2

#if VERSION == 0
// 225.3 ns on i7-6700HQ CPU @ 2.6 GHz running Ubuntu 16.04.01
#include "context_posix.h"
#define context_t          context_posix_t
#define switch_context     switch_context_posix
#define initialize_context initialize_context_posix
#endif

#if VERSION == 1
// 21.1 ns on i7-6700HQ CPU @ 2.6 GHz running Ubuntu 16.04.01
#include "context_fast.h"
#define context_t          context_fast_t
#define switch_context     switch_context_fast
#define initialize_context initialize_context_fast
#endif

#if VERSION == 2
// 12.7 ns on i7-6700HQ CPU @ 2.6 GHz running Ubuntu 16.04.01
#include "context_fastest.h"
#define context_t          context_fastest_t
#define switch_context     switch_context_fastest
#define initialize_context initialize_context_fastest
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>

#ifdef BENCH
#define DEBUG(fmt...)
#else
#define DEBUG(fmt...) printf(fmt)
#endif

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

    DEBUG("done\n");
}

void thread2_fun(void *data)
{
    int i;

    DEBUG("thread2 (%p): -> 1\n", data);
    switch_context(thread2, thread1);

    DEBUG("thread2 (%p): -> 1\n", data);
    switch_context(thread2, thread1);

    DEBUG("thread2 (%p): -> 3\n", data);
    switch_context(thread2, thread3);

    DEBUG("thread2 (%p): -> 1\n", data);
    switch_context(thread2, thread1);

    for (i = 0;; ++i) {
        DEBUG("thread2 (%p): %d -> 3\n", data, i);
        switch_context(thread2, thread3);
    }
}

void thread3_fun(void *data)
{
    int i;

    DEBUG("thread3 (%p): -> 2\n", data);
    switch_context(thread3, thread2);

    DEBUG("thread3 (%p): -> 1\n", data);
    switch_context(thread3, thread1);


    for (i = 0;; ++i) {
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
    void *stack = aligned_malloc(STACKSIZE + RED_ZONE + 8) - 8;
    initialize_context(ctx, stack, STACKSIZE, entry, data);
}

int main()
{
    init_context(thread2, thread2_fun, (void *) 0xDEEEECAF);
    init_context(thread3, thread3_fun, (void *) 0xF000000D);
    thread1_fun((void *) 0xBABEBABE);

    return 0;
}
