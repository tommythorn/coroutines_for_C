        .text
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
#ifdef __riscv
#if __riscv_xlen == 64
        ld      s2,24(s1)
        sd      ra,(s0)
        ld      sp,8(s1)
        ld      a0,16(s1)
        jr      s2
#else // 32-bit
        lw      s2,12(s1)
        sw      ra,(s0)
        lw      sp,4(s1)
        lw      a0,8(s1)
        jr      s2
#endif
#endif

#ifdef __amd64
        popq    0(%rcx)
        movq    %rsp,8(%rcx)
        movq    8(%rdx),%rsp
        movq    16(%rdx),%rdi
        jmp     *24(%rdx)
#endif

        .size   helper_context_fastest,.-helper_context_fastest
