#ifndef _amw_moony_h_
#define _amw_moony_h_

#include <moonlitwalk/defines.h>
#include <moonlitwalk/sewing.h>
#include <moonlitwalk/hadal.h>

AMW_C_DECL_BEGIN

/** An opaque renderer context handle. */
typedef struct amw_moony amw_moony_t;

enum amw_moony_backend {
    AMW_MOONY_ANY_BACKEND = 0,
    AMW_MOONY_BACKEND_VULKAN,
    AMW_MOONY_BACKEND_WEBGPU,
    AMW_MOONY_BACKEND_NULL,
};

enum amw_moony_flags {
    AMW_MOONY_FLAG_INITIALIZED          = 0x1, /* The renderer backend has been initialized, ready to work. */
    AMW_MOONY_FLAG_FRAMEBUFFER_RESIZED  = 0x2, /* If true, the swapchain must be rebuilt. */
};

AMW_API amw_moony_t *  AMW_CALL amw_moony_create(int32_t backend_id, amw_sewing_t *sewing, amw_window_t *window);
AMW_API void           AMW_CALL amw_moony_destroy(amw_moony_t *moony);
AMW_API uint32_t       AMW_CALL amw_moony_flags(amw_moony_t *moony);

AMW_API amw_sewing_t * AMW_CALL amw_moony_sewing(amw_moony_t *moony);
AMW_API amw_window_t * AMW_CALL amw_moony_window(amw_moony_t *moony);

AMW_API void           AMW_CALL amw_moony_begin_frame(amw_moony_t *moony);
AMW_API void           AMW_CALL amw_moony_end_frame(amw_moony_t *moony);

AMW_API void           AMW_CALL amw_moony_framebuffer_resized(amw_moony_t *moony, uint32_t width, uint32_t height);

AMW_C_DECL_END

#endif /* _amw_moony_h_ */
