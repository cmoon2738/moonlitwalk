/****************************************************************************************
 *                                                                                      *
 *  ----------------------------------------------------------------------------------  *
 *  |    0    |    1    |    2    |    3    |    4     |    5    |    6    |    7    |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x0   |   0x4   |   0x8   |   0xc   |   0x10   |   0x14  |   0x18  |   0x1c  |  *
 *  ----------------------------------------------------------------------------------  *
 *  | fc_mxcsr|fc_x87_cw|   EDI   |   ESI   |   EBX    |   EBP   |   EIP   |   EXIT  |  *
 *  ----------------------------------------------------------------------------------  *
 *                                                                                      *
 ****************************************************************************************/

.file "jump_i386_sysv_macho_gas.S"
.text
.globl sew_jump_context
.type sew_jump_context,@function
.align 2

sew_jump_context:
    /* fourth arg of sew_jump_context() == flag indicating preserving FPU */
    movl    0x10(%esp), %ecx

    pushl   %ebp /* save EBP */
    pushl   %ebx /* save EBX */
    pushl   %esi /* save ESI */
    pushl   %edi /* save EDI */

    /* prepare stack for FPU */
    leal    -0x8(%esp), %esp

    /* test for flag preserve_fpu */
    test    %ecx, %ecx
    je      1f

    /* save MMX control-word and status-word */
    stmxcsr (%esp)

    /* save x87 control-word */
    fnstcw  0x4(%esp)

1:
    /* first arg of sew_jump_context() == context jumping from */
    movl    0x1c(%esp), %eax

    /* store ESP (pointing to context-data) in EAX */
    movl    %esp, (%eax)

    /* second arg of sew_jump_context() == context jumping to */
    movl    0x20(%esp), %edx

    /* third arg of sew_jump_context() == value to be returned after jump */
    movl    0x24(%esp), %eax

    /* restore ESP (pointing to context-data) from EDX */
    movl    %edx, %esp

    /* test for flag preserve_fpu */
    test    %ecx, %ecx
    je      2f

    /* restore MMX control-word and status-word */
    ldmxcsr (%esp)

    /* restore x87 control-word */
    fldcw   0x4(%esp)

2:
    /* prepare stack for FPU */
    leal    0x8(%esp), %esp

    popl    %edi /* restore EDI */
    popl    %esi /* restore ESI */
    popl    %ebx /* restore EBX */
    popl    %ebp /* restore EBP */

    /* restore return-address */
    popl    %edx

    /* use value in EAX as return-value after jump
     * use value in EAX as first arg in context function */
    movl    %eax, 0x4(%esp)

    /* indirect jump to context */
    jmp     *%edx
