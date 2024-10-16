#include <moonlitwalk/moony.h>

AMW_C_DECL_BEGIN

#include "vulkan/vk.h"

struct amw_moony {
    uint32_t    flags;

    uint32_t    fb_width, fb_height; /* framebuffer dimensions */
    uint32_t    current_frame;       /* frame index */

    amw_arena_t temporary_arena; /* reset on a function scope */

    amw_window_t *window;
    amw_sewing_t *sewing;

    AMW_MOONY_BACKEND_VK_STATE
};

AMW_C_DECL_END
