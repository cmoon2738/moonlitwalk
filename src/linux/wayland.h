#ifndef _amw_wayland_h_
#define _amw_wayland_h_

#include <moonlitwalk/hadopelagic.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef AMW_NATIVE_WAYLAND
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>
#include <xkbcommon/xkbcommon-compose.h>
#include <xdg-shell-protocol.h>
#include <xdg-output-unstable-v1-protocol.h>
#include <xdg-decoration-unstable-v1-protocol.h>
#include <xdg-activation-v1-protocol.h>
#include <fractional-scale-v1-protocol.h>
#include <pointer-constraints-unstable-v1-protocol.h>
#include <relative-pointer-unstable-v1-protocol.h>
#include <idle-inhibit-unstable-v1-protocol.h>
#include <viewporter-protocol.h>

typedef struct amw_wayland_offer {
    struct wl_data_offer *offer;
    bool                  text_plain_utf8;
    bool                  text_uri_list;
} amw_wayland_offer_t;

typedef struct amw_wayland_scale {
    struct wl_output     *output;
    int32_t               factor;
} amw_wayland_scale_t;

typedef struct amw_wayland_output amw_wayland_output_t;
#define AMW_HADAL_OUTPUT_WAYLAND_STATE amw_wayland_output_t wl;

typedef struct amw_wayland_window amw_wayland_window_t;
#define AMW_HADAL_WINDOW_WAYLAND_STATE amw_wayland_window_t wl;

typedef struct amw_wayland_platform amw_wayland_platform_t;
#define AMW_HADAL_PLATFORM_WAYLAND_STATE amw_wayland_platform_t wl;

struct amw_vidmode;
struct amw_output;

struct amw_wayland_output {
    struct wl_output *output;
    uint32_t          name;
    int32_t           current_mode;
    int32_t           x, y, scale;
};

struct amw_wayland_window {
    struct wl_surface   *surface;
    struct wl_callback  *callback;

    char                *app_id;
    int32_t              width, height;
    int32_t              fb_width, fb_height;

    double cursor_xpos, cursor_ypos;

    struct {
        struct xdg_surface  *surface;
        struct xdg_toplevel *toplevel;
        struct xdg_activation_token_v1     *activation_token;
        struct zxdg_toplevel_decoration_v1 *decoration;
        uint32_t                            decoration_mode;
    } xdg;

    int32_t               buffer_scale;
    amw_wayland_scale_t  *output_scales;
    size_t                output_scale_count;
    size_t                output_scale_size;

    struct wp_viewport            *scaling_viewport;
    uint32_t                       scaling_numerator;
    struct wp_fractional_scale_v1 *fractional_scale;

    struct zwp_relative_pointer_v1 *relative_pointer;
    struct zwp_locked_pointer_v1   *locked_pointer;
    struct zwp_confined_pointer_v1 *confined_pointer;
    struct zwp_idle_inhibitor_v1   *idle_inhibitor;
    struct xdg_activation_token_v1 *activation_token;

    struct {
        int32_t width, height;
        bool    maximized, iconified, activated, fullscreen, resizing;
    } pending;
};

struct amw_wayland_platform {
    const char                 *tag;

    struct wl_display          *display;
    struct wl_registry         *registry;
    struct wl_shm              *shm;
    struct wl_seat             *seat;
    struct wl_compositor       *compositor;
    struct wl_subcompositor    *subcompositor;
    struct wl_pointer          *pointer;
    struct wl_keyboard         *keyboard;
    struct xdg_wm_base         *shell;

    struct wl_data_device_manager          *data_device_manager;
    struct wl_data_device                  *data_device;
    struct wp_viewporter                   *viewporter;

    struct zxdg_decoration_manager_v1      *decoration_manager;
    struct zwp_relative_pointer_manager_v1 *relative_pointer_manager;
    struct zwp_pointer_constraints_v1      *pointer_constraints;
    struct zwp_idle_inhibit_manager_v1     *idle_inhibit_manager;
    struct xdg_activation_v1               *activation_manager;
    struct wp_fractional_scale_manager_v1  *fractional_scale_manager;

    amw_wayland_offer_t     *offers;
    uint32_t                 offer_count;
    struct wl_data_offer    *selection_offer;
    struct wl_data_source   *selection_source;

    struct wl_data_offer    *drag_offer;
    amw_window_t            *drag_focus;
    uint32_t                 drag_serial;

    uint32_t        serial;
    uint32_t        pointer_enter_serial;
    amw_window_t   *pointer_focus;

    int32_t         cursor_timerfd;

    int32_t         key_repeat_timerfd;
    int32_t         key_repeat_rate;
    int32_t         key_repeat_delay;
    int32_t         key_repeat_scancode;
    amw_window_t   *keyboard_focus;

    char           *clipboard_string;
    int16_t         keycodes[256];
    int16_t         scancodes[AMW_KEY_LAST + 1];
    char            keynames[AMW_KEY_LAST + 1][5];

    struct {
        struct xkb_context       *context;
        struct xkb_keymap        *keymap;
        struct xkb_state         *state;
        struct xkb_compose_state *compose_state;

        xkb_mod_index_t           control_index;
        xkb_mod_index_t           alt_index;
        xkb_mod_index_t           shift_index;
        xkb_mod_index_t           super_index;
        xkb_mod_index_t           caps_lock_index;
        xkb_mod_index_t           num_lock_index;
        uint32_t                  modifiers;
    } xkb;

};

/* internal shared code */
bool            hadal_wayland_connect(void);

int32_t         hadal_wayland_init(void);
void            hadal_wayland_terminate(void);

void            hadal_wayland_free_output(struct amw_output *output);
struct amw_vidmode *hadal_wayland_video_modes(struct amw_output *output, int32_t *count);
bool            hadal_wayland_video_mode(struct amw_output *output, struct amw_vidmode *mode);

void            hadal_wayland_get_output_position(struct amw_output *output, int32_t *xpos, int32_t *ypos);
void            hadal_wayland_get_output_content_scale(struct amw_output *output, float *xscale, float *yscale);
void            hadal_wayland_get_output_workarea(struct amw_output *output, int32_t *xpos, int32_t *ypos, int32_t *width, int32_t *height);

void            hadal_wayland_set_clipboard_string(const char *string);
const char     *hadal_wayland_get_clipboard_string(void);

bool            hadal_wayland_create_window(amw_window_t *window, int32_t width, int32_t height);
void            hadal_wayland_destroy_window(amw_window_t *window);
void            hadal_wayland_show_window(amw_window_t *window);
void            hadal_wayland_hide_window(amw_window_t *window);
void            hadal_wayland_retitle_window(amw_window_t *window, const char *title);
void            hadal_wayland_resize_window(amw_window_t *window, int32_t width, int32_t height);
void            hadal_wayland_window_size(amw_window_t *window, int32_t *width, int32_t *height);
void            hadal_wayland_framebuffer_size(amw_window_t *window, int32_t *width, int32_t *height);
void            hadal_wayland_content_scale(amw_window_t *window, float *xscale, float *yscale);

#ifdef AMW_NATIVE_VULKAN
#include <moonlitwalk/vk.h>

bool            hadal_wayland_physical_device_presentation_support(VkPhysicalDevice device, uint32_t queue_family);
VkResult        hadal_wayland_create_surface(VkInstance instance, amw_window_t *window, const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface);

/* internal wayland */
void            hadal_wayland_add_output(uint32_t name, uint32_t version);
void            hadal_wayland_add_seat_listener(struct wl_seat *seat);
void            hadal_wayland_add_data_device_listener(struct wl_data_device *device);
void            hadal_wayland_update_buffer_scale_from_outputs(amw_window_t *window);

#endif /* AMW_NATIVE_VULKAN */


#else
/* null the wayland state */
#define AMW_HADAL_OUTPUT_WAYLAND_STATE
#define AMW_HADAL_WINDOW_WAYLAND_STATE
#define AMW_HADAL_PLATFORM_WAYLAND_STATE
#endif /* AMW_NATIVE_WAYLAND */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _amw_wayland_h_ */
