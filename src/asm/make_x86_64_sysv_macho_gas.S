/****************************************************************************************
 *                                                                                      *
 *  ----------------------------------------------------------------------------------  *
 *  |    0    |    1    |    2    |    3    |    4     |    5    |    6    |    7    |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x0   |   0x4   |   0x8   |   0xc   |   0x10   |   0x14  |   0x18  |   0x1c  |  *
 *  ----------------------------------------------------------------------------------  *
 *  | fc_mxcsr|fc_x87_cw|        R12        |         R13        |        R14        |  *
 *  ----------------------------------------------------------------------------------  *
 *  ----------------------------------------------------------------------------------  *
 *  |    8    |    9    |   10    |   11    |    12    |    13   |    14   |    15   |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x20  |   0x24  |   0x28  |  0x2c   |   0x30   |   0x34  |   0x38  |   0x3c  |  *
 *  ----------------------------------------------------------------------------------  *
 *  |        R15        |        RBX        |         RBP        |        RIP        |  *
 *  ----------------------------------------------------------------------------------  *
 *  ----------------------------------------------------------------------------------  *
 *  |    16   |   17    |                                                            |  *
 *  ----------------------------------------------------------------------------------  *
 *  |   0x40  |   0x44  |                                                            |  *
 *  ----------------------------------------------------------------------------------  *
 *  |        EXIT       |                                                            |  *
 *  ----------------------------------------------------------------------------------  *
 *                                                                                      *
 ****************************************************************************************/

.file "make_x86_64_sysv_macho_gas.S"
.text
.globl _sew_make_context
.align 8

_sew_make_context:
    /* first arg of sew_make_context() == top of context-stack */
    movq    %rdi,   %rax

    /* shift address in RAX to lower 16 byte boundary */
    movabs  $-16,   %r8
    andq    %r8,    %rax

    /* reserve space for context-data on context-stack 
     * size for fc_mxcsr .. RIP + return-address for context-function
     * on context-function entry: (RSP -0x8) % 16 == 0 */
    leaq    -0x48(%rax), %rax

    /* third arg of sew_make_context() == address of context-function */
    movq    %rdx,   0x38(%rax)

    /* save MMX control-word and status-word */
    stmxcsr (%rax)

    /* save x87 control-word */
    fnstcw  0x4(%rax)

    /* compute abs address of label finish */
    leaq    finish(%rip), %rcx

    /* save address of finish as return-address for context-function
     * will be entered after context-function returns */
    movq    %rcx,   0x40(%rax)

    /* return pointer to context-data */
    ret

finish:
    /* exit code is zero */
    xorq    %rdi,   %rdi

    /* exit application */
    call    _exit
    hlt
