#ifndef _amw_hadal_h_
#define _amw_hadal_h_

#include <moonlitwalk/amw.h>
#include <moonlitwalk/system.h>
#include <moonlitwalk/hadopelagic.h>
#include <moonlitwalk/vk.h>

#include "../linux/wayland.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct amw_window {
    uint32_t    flags;
    int32_t     width, height;
    char       *title;

    amw_window_callbacks_t callbacks;
    bool should_close;

    AMW_WINDOW_WAYLAND_STATE
};

typedef struct hadal_api {
    int32_t  id; /* platform */

    int32_t  (*init)(void);
    void     (*terminate)(void);

    bool     (*create_window)(amw_window_t *window);
    void     (*destroy_window)(amw_window_t *window);
    
#ifdef AMW_NATIVE_VULKAN
    bool     (*physical_device_presentation_support)(VkPhysicalDevice device, uint32_t queue_family);
    VkResult (*create_surface)(VkInstance instance, amw_window_t *window, const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface);
#endif /* AMW_NATIVE_VULKAN */
} hadal_api_t;

typedef struct hadopelagic {
    bool initialized;

    hadal_api_t   api;

    amw_mutex_t  *mutex;
    amw_window_t *window_list;
    amw_arena_t   arena;

    AMW_HADOPELAGIC_WAYLAND_STATE
} hadopelagic_t;

/* global platform abstraction, display/input state */
extern hadopelagic_t hadal;

/* callbacks hadal */
void hadal_input_framebuffer_resized_callback(amw_window_t *window, int32_t width, int32_t height);

/* debug hadal */
bool hadal_debug_check_api_uptodate(const hadal_api_t *api);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _amw_hadal_h_ */
