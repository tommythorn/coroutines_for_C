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
#ifdef _amd64
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
#endif

#ifdef __riscv
        // Callee-saves
        // "ra","s0",
        // "a0","a1","a2","a3","a4","a5","a6","a7",
        // "ft0","ft1","ft2","ft3","ft4","ft5","ft6","ft7",
        // "ft8","ft9","ft10","ft11",
        // "fa0","fa1","fa2","fa3","fa4","fa5","fa6","fa7"

        sd      sp,     0(a0)
        sd      ra,     8(a0)
        sd      s0,     16(a0)
        sd      a0,     24(a0)
        sd      a1,     32(a0)
        sd      a2,     40(a0)
        sd      a3,     48(a0)
        sd      a4,     52(a0)
        sd      a5,     64(a0)
        sd      a6,     72(a0)
        sd      a7,     80(a0)
        fsd     ft0,    88(a0)
        fsd     ft1,    96(a0)
        fsd     ft2,    104(a0)
        fsd     ft3,    112(a0)
        fsd     ft4,    120(a0)
        fsd     ft5,    128(a0)
        fsd     ft6,    136(a0)
        fsd     ft7,    144(a0)
        fsd     ft8,    152(a0)
        fsd     ft9,    160(a0)
        fsd     ft10,   168(a0)
        fsd     ft11,   176(a0)
        fsd     fa0,    184(a0)
        fsd     fa1,    192(a0)
        fsd     fa2,    200(a0)
        fsd     fa3,    208(a0)
        fsd     fa4,    216(a0)
        fsd     fa5,    224(a0)
        fsd     fa6,    232(a0)
        fsd     fa7,    232(a0)

        ld      sp,     0(a1)
        ld      ra,     8(a1)
        ld      s0,     16(a1)
        ld      a0,     24(a1)
        ld      a2,     40(a1)
        ld      a3,     48(a1)
        ld      a4,     52(a1)
        ld      a5,     64(a1)
        ld      a6,     72(a1)
        ld      a7,     80(a1)
        fld     ft0,    88(a1)
        fld     ft1,    96(a1)
        fld     ft2,    104(a1)
        fld     ft3,    112(a1)
        fld     ft4,    120(a1)
        fld     ft5,    128(a1)
        fld     ft6,    136(a1)
        fld     ft7,    144(a1)
        fld     ft8,    152(a1)
        fld     ft9,    160(a1)
        fld     ft10,   168(a1)
        fld     ft11,   176(a1)
        fld     fa0,    184(a1)
        fld     fa1,    192(a1)
        fld     fa2,    200(a1)
        fld     fa3,    208(a1)
        fld     fa4,    216(a1)
        fld     fa5,    224(a1)
        fld     fa6,    232(a1)
        fld     fa7,    232(a1)
        ld      a1,     32(a1)
        ret
#endif
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
#ifdef _amd64
        movq    %rbx,%rdi
        callq   *%r12
        mov     $1,%rdi
        callq   _exit
#endif



        .size   helper_context_fastest,.-helper_context_fastest
