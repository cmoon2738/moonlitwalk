#ifndef _amw_sew_context_h_
#define _amw_sew_context_h_

#include <moonlitwalk/amw.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* internal sewing fiber context */
typedef void *sew_context_t;

/* implemented in asm */
intptr_t sew_jump_context(sew_context_t *ofc, sew_context_t nfc, intptr_t vp, int preserve_fpu);

/* implemented in asm, sp is the pointer to the top of the stack (&stack_buffer[size]) */
sew_context_t sew_make_context(void *sp, size_t size, void (*fn)(intptr_t));

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _amw_sew_context_h_ */
