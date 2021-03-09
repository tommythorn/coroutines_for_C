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
        // Callee-saves (ra is special)
	// sp, s0, .., s11
	// fs0, .., fs11

        sd      ra,     0(a0)
        sd      sp,     8(a0)
        sd      s0,     16(a0)
        sd      s1,     24(a0)
        sd      s2,     32(a0)
        sd      s3,     40(a0)
        sd      s4,     48(a0)
        sd      s5,     56(a0)
        sd      s6,     64(a0)
        sd      s7,     72(a0)
        sd      s8,     80(a0)
        sd      s9,     88(a0)
        sd      s10,    96(a0)
        sd      s11,    104(a0)
        fsd     fs0,    112(a0)
        fsd     fs1,    120(a0)
        fsd     fs2,    128(a0)
        fsd     fs3,    136(a0)
        fsd     fs4,    144(a0)
        fsd     fs5,    152(a0)
        fsd     fs6,    160(a0)
        fsd     fs7,    168(a0)
        fsd     fs8,    176(a0)
        fld     fs9,    184(a0)
        fld     fs10,   192(a0)
        fld     fs11,   200(a0)
	
        ld      ra,     0(a1)
        ld      sp,     8(a1)
        ld      s0,     16(a1)
        ld      s1,     24(a1)
        ld      s2,     32(a1)
        ld      s3,     40(a1)
        ld      s4,     48(a1)
        ld      s5,     56(a1)
        ld      s6,     64(a1)
        ld      s7,     72(a1)
        ld      s8,     80(a1)
        ld      s9,     88(a1)
        ld      s10,    96(a1)
        ld      s11,    104(a1)
        fld     fs0,    112(a1)
        fld     fs1,    120(a1)
        fld     fs2,    128(a1)
        fld     fs3,    136(a1)
        fld     fs4,    144(a1)
        fld     fs5,    152(a1)
        fld     fs6,    160(a1)
        fld     fs7,    168(a1)
        fld     fs8,    176(a1)
        fld     fs9,    184(a1)
        fld     fs10,   192(a1)
        fld     fs11,   200(a1)
        ret // XXX replace with jalr t0, ra to get slightly better RAS behavior
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
	ret
#endif

#ifdef __riscv
	ld	t0, 208(a1)
	ld	a0, 216(a1)
	jr	t0
#endif

        .size   helper_context_fastest,.-helper_context_fastest
