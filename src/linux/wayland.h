#ifndef _amw_wayland_h_
#define _amw_wayland_h_

#include "../hadopelagic.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef AMW_NATIVE_WAYLAND
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include <xdg-shell-protocol.h>

typedef struct amw_wayland_window {
    struct xdg_surface  *surface;
    struct xdg_toplevel *toplevel;
} amw_wayland_window_t;
#define AMW_WINDOW_WAYLAND_STATE amw_wayland_window_t wl;

typedef struct amw_wayland {
    struct wl_display   *display;
    struct wl_registry  *registry;
    struct wl_seat      *seat;
    struct wl_shm       *shm;
    struct xdg_wm_base  *shell;
} amw_wayland_t;
extern amw_wayland_t wl;

int32_t     amw_wayland_connect(void);

int32_t     amw_wayland_init(void);
void        amw_wayland_terminate(void);

bool        amw_wayland_window_create(amw_window_t *window);
void        amw_wayland_window_destroy(amw_window_t *window);
VkResult    amw_wayland_vk_surface_create(amw_window_t *window, VkInstance instance, const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface);
#else
    /* null the wayland state */
    #define AMW_WINDOW_WAYLAND_STATE 
#endif /* AMW_NATIVE_WAYLAND */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _amw_wayland_h_ */
