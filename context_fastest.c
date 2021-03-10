#include "context_fastest.h"

#include <stdint.h>
#include <string.h>
#include <assert.h>

void helper_context_fastest(void);

void initialize_context_fastest(context_fastest_t ctx,
                                void *stack_base,
                                size_t stack_size,
                                void (*entry)(void *data),
                                void *data)
{
    // XXX Caveat, only valid for AMD64 ABI (and RISC-V?)
    // See fx. https://software.intel.com/en-us/forums/intel-isa-extensions/topic/291241
#define RED_ZONE 128
    uintptr_t stack_end = (uintptr_t)stack_base + stack_size - RED_ZONE;
    stack_end &= -16;  // ensure that the stack is 16-byte aligned

#ifdef _amd64
    uint64_t *sp = (uint64_t *)stack_end;

    *--sp = (uint64_t) helper_context_fastest;
    *--sp = (uint64_t) data; // %rbx
    *--sp = 0; // %rbp
    *--sp = (uint64_t) entry; // %r12
    *--sp = 0; // %r13
    *--sp = 0; // %r14
    *--sp = 0; // %r15
    ctx->sp = sp;
#endif

#ifdef __riscv
    memset(ctx, 0, sizeof *ctx);
    ctx->sp = stack_end;
    ctx->ra = (uintptr_t) helper_context_fastest;
    ctx->data = data;
    ctx->entry = entry;
#endif
}
