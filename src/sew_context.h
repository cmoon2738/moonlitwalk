#ifndef _amw_sew_context_h_
#define _amw_sew_context_h_

#include <moonlitwalk/defines.h>

AMW_C_DECL_BEGIN

/* internal sewing fiber context */
typedef void *sew_context_t;

/* implemented in asm, ofc/nfc - original/new fiber context */
extern intptr_t AMW_CALL sew_jump_context(sew_context_t *ofc, sew_context_t nfc, intptr_t vp, int preserve_fpu);

/* implemented in asm, sp is the pointer to the top of the stack (&stack_buffer[size]) */
extern sew_context_t AMW_CALL sew_make_context(void *sp, size_t size, void (*fn)(intptr_t));

AMW_C_DECL_END

#endif /* _amw_sew_context_h_ */
