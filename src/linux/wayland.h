#ifndef _amw_wayland_h_
#define _amw_wayland_h_

#include <moonlitwalk/hadopelagic.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef AMW_NATIVE_WAYLAND
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include <xdg-shell-protocol.h>

typedef struct amw_wayland_window {
    struct wl_surface   *surface;

    struct {
        struct xdg_surface  *surface;
        struct xdg_toplevel *toplevel;
    } xdg;
} amw_wayland_window_t;
#define AMW_WINDOW_WAYLAND_STATE amw_wayland_window_t wl;

typedef struct amw_wayland {
    struct wl_display   *display;
    struct wl_registry  *registry;
    struct wl_seat      *seat;
    struct wl_shm       *shm;
    struct xdg_wm_base  *shell;
} amw_wayland_t;
#define AMW_HADOPELAGIC_WAYLAND_STATE amw_wayland_t wl;

bool        hadal_wayland_connect(void);

int32_t     hadal_wayland_init(void);
void        hadal_wayland_terminate(void);

bool        hadal_wayland_create_window(amw_window_t *window);
void        hadal_wayland_destroy_window(amw_window_t *window);

#ifdef AMW_NATIVE_VULKAN
#include <moonlitwalk/vk.h>

bool        hadal_wayland_physical_device_presentation_support(VkPhysicalDevice device, uint32_t queue_family);
VkResult    hadal_wayland_create_surface(VkInstance instance, amw_window_t *window, const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface);
#endif /* AMW_NATIVE_VULKAN */

#else
    /* null the wayland state */
    #define AMW_WINDOW_WAYLAND_STATE 
    #define AMW_HOPELAGIC_WAYLAND_STATE 
#endif /* AMW_NATIVE_WAYLAND */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _amw_wayland_h_ */
