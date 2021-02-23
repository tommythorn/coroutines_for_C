#include "context_posix.h"

void switch_context_posix(context_posix_t from,
                          context_posix_t to)
{
    swapcontext(&from->ucontext, &to->ucontext);
}

void initialize_context_posix(context_posix_t ctx,
                              void *stack_base,
                              size_t stack_size,
                              void (*entry)(void *data),
                              void *data)
{
    getcontext(&ctx->ucontext);
    ctx->ucontext.uc_stack.ss_sp = stack_base;
    ctx->ucontext.uc_stack.ss_size = stack_size;
    ctx->ucontext.uc_link = 0;
    makecontext(&ctx->ucontext, (void(*)()) entry, 1, data);
}
