        .text
        .align  4
        .globl  switch_context_fastest
        .type   switch_context_fastest, @function

/*
        All we have to save is the callee saved registers.  Everything
        is dumped to the stack and the only thing stored in, and
        restored from, the context is the stack pointer.

*/
switch_context_fastest:
        push    %rbx
        push    %rbp
        push    %r12
        push    %r13
        push    %r14
        push    %r15
        movq    %rsp,(%rdi)
        movq    (%rsi),%rsp
        pop     %r15
        pop     %r14
        pop     %r13
        pop     %r12
        pop     %rbp
        pop     %rbx
        ret

        .size   switch_context_fastest,.-switch_context_fastest
/*
        The helper function is the first to be entered in a new context
        and serves to call the user entry function with the correct
        argument.  The reason we need a helper is that user entry
        function argument isn't one of the saved registers

        Our only job is to collect the entry and data from the registers and call it.
        We also exit if the user entry should ever return (which it isn't supposed to).
*/

        .align  4
        .globl  helper_context_fastest
        .type   helper_context_fastest, @function
helper_context_fastest:
        movq    %rbx,%rdi
        callq   *%r12
        mov     $1,%rdi
        callq   _exit

        .size   helper_context_fastest,.-helper_context_fastest
