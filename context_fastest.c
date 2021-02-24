#include "context_fastest.h"

#include <stdint.h>

void helper_context_fastest(void);

void initialize_context_fastest(context_fastest_t ctx,
                                void *stack_base,
                                size_t stack_size,
                                void (*entry)(void *data),
                                void *data)
{
    // XXX Caveat, only valid for AMD64 ABI
    // See fx. https://software.intel.com/en-us/forums/intel-isa-extensions/topic/291241
#define RED_ZONE 128
    uintptr_t stack_end = (uintptr_t)stack_base + stack_size - RED_ZONE;
    stack_end &= -16;  // ensure that the stack is 16-byte aligned
    stack_end -= 8; // to account for the pop
    ctx->resume_pc = helper_context_fastest;
    ctx->main_func = entry;
    ctx->sp = (void *)stack_end;
    ctx->data = data;
}
