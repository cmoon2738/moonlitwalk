#ifndef _amw_internal_h_
#define _amw_internal_h_

#include "amw.h"
#include "system.h"
#include "hadopelagic.h"

#include "linux/wayland.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct amw_window {
    uint32_t    flags;
    int32_t     width, height;
    char       *title;

    bool should_close;

    AMW_WINDOW_WAYLAND_STATE
};

typedef struct hadal_api {
    int32_t  id; /* platform */

    int32_t  (*init)(void);
    void     (*terminate)(void);

    bool     (*window_create)(amw_window_t *window);
    void     (*window_destroy)(amw_window_t *window);
    
    bool     (*physical_device_presentation_support)(VkInstance instance, VkPhysicalDevice device, uint32_t queue_family);
    VkResult (*surface_create)(VkInstance instance, amw_window_t *window, const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface);
} hadal_api_t;

typedef struct hadopelagic {
    bool initialized;

    hadal_api_t   api;

    amw_mutex_t  *mutex;
    amw_window_t *window_list;

    AMW_HADAL_WAYLAND_STATE
} hadopelagic_t;

/* global platform abstraction, display/input state */
extern hadopelagic_t hadal;

bool hadal_debug_check_api_uptodate(const hadal_api_t *api);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _amw_hadopelagic_h_ */
