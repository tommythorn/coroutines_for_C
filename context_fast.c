#include "context_fast.h"

typedef struct helper_args_s {
    void(*      fnc)(void*);
    void*       ctx;
    jmp_buf*    cur;
    ucontext_t* prv;
} *helper_args_t;

void switch_context_fast(context_fast_t from,
                         context_fast_t to)
{
    if (_setjmp(from->jmp) == 0)
        _longjmp(to->jmp, 1);
}

static void
helper_context_fast(void *p)
{
    helper_args_t args = (helper_args_t)p;
    void (*ufnc)(void*) = args->fnc;
    void *uctx = args->ctx;
    ucontext_t tmp;

    if (_setjmp(*args->cur) == 0)
        swapcontext(&tmp, args->prv);

    ufnc(uctx);
}

void initialize_context_fast(context_fast_t ctx,
                             void *stack_base,
                             size_t stack_size,
                             void (*entry)(void *data),
                             void *data)
{
    getcontext(&ctx->ucontext);
    ctx->ucontext.uc_stack.ss_sp = stack_base;
    ctx->ucontext.uc_stack.ss_size = stack_size;
    ctx->ucontext.uc_link = 0;
    ucontext_t tmp;
    struct helper_args_s args = {entry, data, &ctx->jmp, &tmp};
    makecontext(&ctx->ucontext, (void(*)()) helper_context_fast, 1, &args);
    swapcontext(&tmp, &ctx->ucontext);
}
