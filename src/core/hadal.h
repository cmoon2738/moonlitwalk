#ifndef _amw_hadal_h_
#define _amw_hadal_h_

#include <moonlitwalk/amw.h>
#include <moonlitwalk/system.h>
#include <moonlitwalk/hadopelagic.h>
#include <moonlitwalk/vk.h>

#include "../linux/wayland.h"

#ifdef AMW_PLATFORM_UNIX
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define HADAL_CONNECTED     8
#define HADAL_DISCONNECTED 16
#define HADAL_INSERT_FIRST 32
#define HADAL_INSERT_LAST  64
#define HADAL_DONT_CARE    -1

#define HADAL_CURSOR_DISABLED 0

typedef struct amw_vidmode {
    int32_t width;
    int32_t height;
    int32_t red_bits;
    int32_t green_bits;
    int32_t blue_bits;
    int32_t refresh_rate;
} amw_vidmode_t;

typedef struct amw_output {
    char        name[128];
    int32_t     width_mm, height_mm;

    amw_vidmode_t *modes;
    int32_t        mode_count;
    amw_vidmode_t  current_mode;

    /* window whose video mode is current on this output */
    amw_window_t *window;

    AMW_HADAL_OUTPUT_WAYLAND_STATE
} amw_output_t;

struct amw_window {
    amw_window_t *next;
    char         *title;

    bool          visible;
    bool          resizable;
    bool          auto_iconify;
    bool          iconified;
    bool          maximized;
    bool          activated;
    bool          fullscreen;
    bool          transparent;
    bool          decorated;
    bool          hovered;
    bool          focused;
    bool          focus_on_show;
    bool          scale_framebuffer;
    bool          should_close;

    int32_t       minwidth, minheight;
    int32_t       maxwidth, maxheight;
    int32_t       numer, denom; /* TODO aspect ratio */

    /* current output bound to this window */
    amw_output_t *output;
    amw_vidmode_t vidmode;

    /* TODO joystick, touch, mouse cursor */
    char mouse_buttons[AMW_MOUSE_BUTTON_LAST + 1];
    int32_t cursor_mode;

    bool sticky_keys;
    bool lock_key_mods;
    char keys[AMW_KEY_LAST + 1];

    amw_input_callbacks_t    input_callbacks;
    amw_renderer_callbacks_t renderer_callbacks;

    AMW_HADAL_WINDOW_WAYLAND_STATE
};

typedef struct hadal_api {
    int32_t    id; /* platform */

    int32_t  (*init)(void);
    void     (*terminate)(void);

    void           (*free_output)(amw_output_t *output);
    amw_vidmode_t *(*video_modes)(amw_output_t *output, int32_t *count);
    bool           (*video_mode)(amw_output_t *output, amw_vidmode_t *mode);
    void           (*get_output_position)(amw_output_t *output, int32_t *xpos, int32_t *ypos);
    void           (*get_output_content_scale)(amw_output_t *output, float *xscale, float *yscale);
    void           (*get_output_workarea)(amw_output_t *output, int32_t *xpos, int32_t *ypos, int32_t *width, int32_t *height);
    void           (*set_clipboard_string)(const char *string);
    const char    *(*get_clipboard_string)(void);

    bool     (*create_window)(amw_window_t *window, int32_t width, int32_t height);
    void     (*destroy_window)(amw_window_t *window);
    void     (*show_window)(amw_window_t *window);
    void     (*hide_window)(amw_window_t *window);
    void     (*retitle_window)(amw_window_t *window, const char *title);
    void     (*resize_window)(amw_window_t *window, int32_t width, int32_t height);
    void     (*window_size)(amw_window_t *window, int32_t *width, int32_t *height);
    void     (*framebuffer_size)(amw_window_t *window, int32_t *width, int32_t *height);
    void     (*content_scale)(amw_window_t *window, float *xscale, float *yscale);

#ifdef AMW_NATIVE_VULKAN
    bool     (*physical_device_presentation_support)(VkPhysicalDevice device, uint32_t queue_family);
    VkResult (*create_surface)(VkInstance instance, amw_window_t *window, const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface);
#endif /* AMW_NATIVE_VULKAN */
} hadal_api_t;

typedef struct hadopelagic {
    bool initialized;

    hadal_api_t     api;
    amw_arena_t     temp_arena;      
    amw_arena_t     clipboard_arena; /* reset every clipboard change */

    amw_mutex_t    *mutex;
    amw_window_t   *window_list;

    amw_output_t  **outputs;
    int32_t         output_count;

    AMW_HADAL_PLATFORM_WAYLAND_STATE
} hadopelagic_t;

/* global platform abstraction, display/input state */
extern hadopelagic_t hadal;

/* internal shared calls */
void          hadal_input_output(amw_output_t *output, int32_t action, int32_t placement);
amw_output_t *hadal_alloc_output(const char *name, int32_t width_mm, int32_t height_mm);
void          hadal_free_output(amw_output_t *output);
const amw_vidmode_t *hadal_choose_video_mode(amw_output_t *output, const amw_vidmode_t *desired);
char        **hadal_parse_uri_list(char *text, int32_t *count);

#ifdef AMW_PLATFORM_UNIX
bool          hadal_poll_posix(struct pollfd *fds, nfds_t count, double *timeout);
#endif

/* callbacks hadal */
void hadal_input_key_callback(amw_window_t *window, int32_t key, int32_t scancode, int32_t action, int32_t mods);
void hadal_input_framebuffer_resized_callback(amw_window_t *window, int32_t width, int32_t height);

/* debug hadal */
bool hadal_debug_check_api_uptodate(const hadal_api_t *api);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _amw_hadal_h_ */
