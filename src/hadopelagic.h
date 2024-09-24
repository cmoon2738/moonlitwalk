#ifndef _amw_hadopelagic_h_
#define _amw_hadopelagic_h_

#include "amw.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef enum amw_platform_id {
    AMW_HADAL_ANY_PLATFORM = 0,
    AMW_HADAL_PLATFORM_WIN32,
    AMW_HADAL_PLATFORM_COCOA,
    AMW_HADAL_PLATFORM_IOS,
    AMW_HADAL_PLATFORM_ANDROID,
    AMW_HADAL_PLATFORM_WAYLAND,
    AMW_HADAL_PLATFORM_XCB,
    AMW_HADAL_PLATFORM_KMS,
    AMW_HADAL_PLATFORM_HEADLESS,
} amw_platform_id_t;

typedef struct amw_window amw_window_t;

typedef enum amw_window_flags {
    AMW_WINDOW_FLAG_TODO = 0x00000001,
} amw_window_flags_t;

typedef struct amw_window_callbacks {
    void *rendering_data;

    void *(*framebuffer_resized)(void *data, amw_window_t *window, int32_t width, int32_t height);
} amw_window_callbacks_t;

uint32_t      amw_hadal_platform(void);
bool          amw_hadal_platform_supported(int32_t id);

int32_t       amw_hadal_init(int32_t platform_id);
void          amw_hadal_terminate(void);

amw_window_t *amw_hadal_create_window(const char *title, int32_t width, int32_t height, uint32_t flags);
void          amw_hadal_destroy_window(amw_window_t *window);
bool          amw_hadal_should_close(amw_window_t *window);
void          amw_hadal_request_close(amw_window_t *window, bool request);

void          amw_hadal_setup_callbacks(amw_window_t *window, amw_window_callbacks_t *callbacks);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _amw_hadopelagic_h_ */
