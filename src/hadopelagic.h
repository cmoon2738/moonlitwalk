#ifndef _amw_hadopelagic_h_
#define _amw_hadopelagic_h_

#include <moonlitwalk/os.h>
#include <moonlitwalk/hadal.h>

#include "linux/linux_joystick.h"
#include "linux/wl.h"

#ifdef AMW_PLATFORM_UNIX
    #include <unistd.h>
    #include <poll.h>
#endif /* AMW_PLATFORM_UNIX */

AMW_C_DECL_BEGIN

#define HADAL_CONNECTED     8
#define HADAL_DISCONNECTED 16
#define HADAL_INSERT_FIRST 32
#define HADAL_INSERT_LAST  64

struct amw_output {
    char           name[128];
    uint32_t       width_mm, height_mm;

    amw_vidmode_t *modes;
    int32_t        mode_count;
    amw_vidmode_t  current_mode;

    /* window whose video mode is current on this output, for fullscreen */
    amw_window_t  *window;
    /* pointer to user data */
    void *         data;

    AMW_HADAL_OUTPUT_WAYLAND_STATE
};

struct amw_cursor {
    amw_cursor_t *next;
    AMW_HADAL_CURSOR_WAYLAND_STATE
};

typedef struct js_map {
    uint8_t     type;
    uint8_t     index;
    int8_t      axis_scale;
    int8_t      axis_offset;
} js_map_t;

typedef struct js_mapping {
    char        name[128];
    char        guid[33];
    js_map_t    buttons[15];
    js_map_t    axes[6];
} js_mapping_t;

typedef struct amw_joystick {
    bool        allocated, connected;
    char        name[128];

    float      *axes;
    int32_t     axis_count;

    uint8_t    *buttons;
    int32_t     button_count;

    uint8_t    *hats;
    int32_t     hat_count;

    js_mapping_t *mapping;

    AMW_HADAL_JOYSTICK_LINUX_STATE
} amw_joystick_t;

struct amw_window {
    amw_window_t *next;

    char         *title;
    uint32_t      flags;

    uint32_t      minwidth, minheight;
    uint32_t      maxwidth, maxheight;
    uint32_t      numer, denom;

    /* current output monitor bound to this window, for fullscreen */
    amw_output_t *output;
    amw_cursor_t *cursor;
    amw_vidmode_t vidmode;

    amw_arena_t   title_arena;

    /* TODO joystick, touch, mouse cursor */
    int32_t cursor_mode;
    char    mouse_buttons[AMW_MOUSE_BUTTON_LAST + 1];
    char    keys[AMW_KEY_LAST + 1];

    AMW_HADAL_WINDOW_WAYLAND_STATE
};

typedef struct hadopelagic_api_t {
    int32_t        platform_id;

    /* main backend */
    int32_t        (AMW_CALL *init)(void);
    void           (AMW_CALL *terminate)(void);

    /* windowing system calls */
    void           (AMW_CALL *set_clipboard_string)(const char *string);
    const char    *(AMW_CALL *get_clipboard_string)(void);

    /* output monitor calls */
    void           (AMW_CALL *free_output)(amw_output_t *output);
    amw_vidmode_t *(AMW_CALL *video_modes)(amw_output_t *output, int32_t *count);
    bool           (AMW_CALL *video_mode)(amw_output_t *output, amw_vidmode_t *mode);
    void           (AMW_CALL *get_output_position)(amw_output_t *output, int32_t *xpos, int32_t *ypos);
    void           (AMW_CALL *get_output_content_scale)(amw_output_t *output, float *xscale, float *yscale);
    void           (AMW_CALL *get_output_workarea)(amw_output_t *output, int32_t *xpos, int32_t *ypos, uint32_t *width, uint32_t *height);

    /* window state calls */
    bool           (AMW_CALL *create_window)(amw_window_t *window, uint32_t width, uint32_t height);
    void           (AMW_CALL *destroy_window)(amw_window_t *window);
    void           (AMW_CALL *retitle_window)(amw_window_t *window, const char *title);
    void           (AMW_CALL *resize_window)(amw_window_t *window, uint32_t width, uint32_t height);
    void           (AMW_CALL *window_size)(amw_window_t *window, uint32_t *width, uint32_t *height);
    void           (AMW_CALL *framebuffer_size)(amw_window_t *window, uint32_t *width, uint32_t *height);
    void           (AMW_CALL *content_scale)(amw_window_t *window, float *xscale, float *yscale);

    /* window flag property calls */
    void           (AMW_CALL *show_window)(amw_window_t *window);
    void           (AMW_CALL *hide_window)(amw_window_t *window);

#ifdef AMW_NATIVE_VULKAN
    /* vulkan calls */
    bool           (AMW_CALL *physical_device_presentation_support)(amw_window_t *window, VkPhysicalDevice device, uint32_t queue_family);
    VkResult       (AMW_CALL *create_surface)(VkInstance instance, amw_window_t *window, const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface);
#endif /* AMW_NATIVE_VULKAN */

} hadopelagic_api_t;

typedef struct hadopelagic {
    bool                initialized;
    hadopelagic_api_t   api;

    amw_arena_t         clipboard_arena; /* reset every clipboard change */

    amw_mutex_t        *mutex;
    amw_window_t       *window_list_head;
    amw_cursor_t       *cursor_list_head;

    amw_output_t      **outputs;
    int32_t             output_count;

    AMW_HADAL_HADOPELAGIC_WAYLAND_STATE

    bool                joysticks_initialized;
    amw_joystick_t      joysticks[AMW_JOYSTICK_LAST + 1];
    js_mapping_t       *mappings;
    int32_t             mapping_count;

    AMW_HADAL_HADOPELAGIC_JOYSTICK_LINUX_STATE
} hadopelagic_t;

/* global hadopelagic state */
extern hadopelagic_t hadal;

/* internal shared calls */
extern void                  AMW_CALL hadal_connect_output(amw_output_t *output, int32_t action, int32_t placement);
extern amw_output_t *        AMW_CALL hadal_alloc_output(const char *name, uint32_t width_mm, uint32_t height_mm);
extern void                  AMW_CALL hadal_free_output(amw_output_t *output);
extern const amw_vidmode_t * AMW_CALL hadal_choose_video_mode(amw_output_t *output, const amw_vidmode_t *desired);
extern char **               AMW_CALL hadal_parse_uri_list(char *text, int32_t *count);

#ifdef AMW_PLATFORM_UNIX
extern bool                  AMW_CALL hadal_poll_posix(struct pollfd *fds, nfds_t count, double *timeout);
#elif defined(AMW_PLATFORM_WINDOWS)
#endif

/* debug hadal */
extern bool                  AMW_CALL hadal_debug_check_api_uptodate(const hadopelagic_api_t *api);

AMW_C_DECL_END

#endif /* _amw_hadopelagic_h_ */
