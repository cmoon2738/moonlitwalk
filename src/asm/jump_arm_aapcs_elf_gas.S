/*******************************************************
 *                                                     *
 *  -------------------------------------------------  *
 *  |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  *
 *  -------------------------------------------------  *
 *  | 0x0 | 0x4 | 0x8 | 0xc | 0x10| 0x14| 0x18| 0x1c|  *
 *  -------------------------------------------------  *
 *  | s16 | s17 | s18 | s19 | s20 | s21 | s22 | s23 |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  8  |  9  |  10 |  11 |  12 |  13 |  14 |  15 |  *
 *  -------------------------------------------------  *
 *  | 0x20| 0x24| 0x28| 0x2c| 0x30| 0x34| 0x38| 0x3c|  *
 *  -------------------------------------------------  *
 *  | s24 | s25 | s26 | s27 | s28 | s29 | s30 | s31 |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  16 |  17 |  18 |  19 |  20 |  21 |  22 |  23 |  *
 *  -------------------------------------------------  *
 *  | 0x40| 0x44| 0x48| 0x4c| 0x50| 0x54| 0x58| 0x5c|  *
 *  -------------------------------------------------  *
 *  |  v1 |  v2 |  v3 |  v4 |  v5 |  v6 |  v7 |  v8 |  *
 *  -------------------------------------------------  *
 *  -------------------------------------------------  *
 *  |  24 |  25 |                                   |  *
 *  -------------------------------------------------  *
 *  | 0x60| 0x64|                                   |  *
 *  -------------------------------------------------  *
 *  |  lr |  pc |                                   |  *
 *  -------------------------------------------------  *
 *                                                     *
 *******************************************************/

.file "jump_arm_aapcs_elf_gas.S"
.text
.globl sew_jump_context
.type sew_jump_context,@function
.align 2

sew_jump_context:
    @ save LR as PC
    push    {lr}
    @ save V1-V8, LR
    push    {v1-v8, lr}

    @ prepare stack for FPU
    sub     sp, sp, #64

#if (defined(__VFP_FP__) && !defined(__SOFTFP__))
    @ test if fpu env should be preserved
    cmp     a4, #0
    beq     1f

    @ save S16-S31
    vstmia  sp, {d8-d15}

1:
#endif

    @ store RSP (pointing to context-data) in A1
    str     sp, [a1]

    @ restore RSP (pointing to context-data) from A2
    mov     sp, a2

#if (defined(__VFP_FP__) && !defined(__SOFTFP__))
    @ test if fpu env should be preserved
    cmp     a4, #0
    beq     2f
    
    @ restore S16-S31
    vldmia  sp, {d8-d15}

2:
#endif

    @ prepare stack for FPU
    add     sp, sp, #64

    @ use third arg as return value after jump
    @ and as first arg in context function
    mov     a1, a3

    @ restore V1-V8, LR, PC
    pop     {v1-v8, lr, pc}
.size sew_jump_context,.-sew_jump_context

@ mark that we don't need executable stack
.section .note.GNU-stack,"",%progbits
