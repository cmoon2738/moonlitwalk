#include "../core/hadal.h"
#include "wayland.h"

#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <sys/timerfd.h>
#include <unistd.h>

static void wm_base_handle_ping(void *data,
                                struct xdg_wm_base *shell,
                                uint32_t serial)
{
    /* unused */
    (void)data;

    xdg_wm_base_pong(shell, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = wm_base_handle_ping,
};

static void handle_registry_global(void *data, 
                                   struct wl_registry *registry, 
                                   uint32_t name, 
                                   char const *interface, 
                                   uint32_t version)
{
    /* unused */
    (void)data;

    if (strcmp(interface, "wl_compositor") == 0) {
        hadal.wl.compositor = wl_registry_bind(registry, name, &wl_compositor_interface, amw_min(3, version));

    } else if (strcmp(interface, "wl_subcompositor") == 0) {
        hadal.wl.subcompositor = wl_registry_bind(registry, name, &wl_subcompositor_interface, 1);

    } else if (strcmp(interface, "wl_shm") == 0) {
        hadal.wl.shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);

    } else if (strcmp(interface, "wl_output") == 0) {
        hadal_wayland_add_output(name, version);

    } else if (strcmp(interface, "wl_seat") == 0) {
        if (!hadal.wl.seat) {
            hadal.wl.seat = wl_registry_bind(registry, name, &wl_seat_interface, amw_min(4, version));
            hadal_wayland_add_seat_listener(hadal.wl.seat);

            if (wl_seat_get_version(hadal.wl.seat) >= WL_KEYBOARD_REPEAT_INFO_SINCE_VERSION)
                hadal.wl.key_repeat_timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
        }
    } else if (strcmp(interface, "wl_data_device_manager") == 0) {
        if (!hadal.wl.data_device_manager)
            hadal.wl.data_device_manager = wl_registry_bind(registry, name, &wl_data_device_manager_interface, 1);

    } else if (strcmp(interface, "xdg_wm_base") == 0) {
        hadal.wl.shell = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(hadal.wl.shell, &xdg_wm_base_listener, NULL);

    } else if (strcmp(interface, "zxdg_decoration_manager_v1") == 0) {
        hadal.wl.decoration_manager = wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1);

    } else if (strcmp(interface, "zwp_relative_pointer_manager_v1") == 0) {
        hadal.wl.relative_pointer_manager = wl_registry_bind(registry, name, &zwp_relative_pointer_manager_v1_interface, 1);

    } else if (strcmp(interface, "zwp_pointer_constraints_v1") == 0) {
        hadal.wl.pointer_constraints = wl_registry_bind(registry, name, &zwp_pointer_constraints_v1_interface, 1);

    } else if (strcmp(interface, "zwp_idle_inhibit_manager_v1") == 0) {
        hadal.wl.idle_inhibit_manager = wl_registry_bind(registry, name, &zwp_idle_inhibit_manager_v1_interface, 1);

    } else if (strcmp(interface, "xdg_activation_v1") == 0) {
        hadal.wl.activation_manager = wl_registry_bind(registry, name, &xdg_activation_v1_interface, 1);

    } else if (strcmp(interface, "wp_viewporter") == 0) {
        hadal.wl.viewporter = wl_registry_bind(registry, name, &wp_viewporter_interface, 1);

    /* FIXME the fractional scale crashes at 'creating native surface', will have to fix it
    } else if (strcmp(interface, "wp_fractional_scale_manager_v1") == 0) {
        hadal.wl.fractional_scale_manager = wl_registry_bind(registry, name, &wp_fractional_scale_manager_v1_interface, 1);
    */
    }
}

static void handle_registry_global_remove(void *data, 
                                          struct wl_registry *registry, 
                                          uint32_t name)
{
    /* unused */
    (void)data;
    (void)registry;

    for (int32_t i = 0; i < hadal.output_count; i++) {
        amw_output_t *output = hadal.outputs[i];
        if (output->wl.name == name) {
            hadal_create_output(output, HADAL_DISCONNECTED, 0);
            return;
        }
    }
}

static const struct wl_registry_listener registry_listener = {
    .global        = handle_registry_global,
    .global_remove = handle_registry_global_remove,
};

/* key code translation table */
static void create_key_tables(void)
{
    amw_memset(hadal.wl.keycodes, -1, sizeof(hadal.wl.keycodes));
    amw_memset(hadal.wl.scancodes, -1, sizeof(hadal.wl.scancodes));

    hadal.wl.keycodes[KEY_GRAVE]      = AMW_KEY_GRAVE_ACCENT;
    hadal.wl.keycodes[KEY_1]          = AMW_KEY_1;
    hadal.wl.keycodes[KEY_2]          = AMW_KEY_2;
    hadal.wl.keycodes[KEY_3]          = AMW_KEY_3;
    hadal.wl.keycodes[KEY_4]          = AMW_KEY_4;
    hadal.wl.keycodes[KEY_5]          = AMW_KEY_5;
    hadal.wl.keycodes[KEY_6]          = AMW_KEY_6;
    hadal.wl.keycodes[KEY_7]          = AMW_KEY_7;
    hadal.wl.keycodes[KEY_8]          = AMW_KEY_8;
    hadal.wl.keycodes[KEY_9]          = AMW_KEY_9;
    hadal.wl.keycodes[KEY_0]          = AMW_KEY_0;
    hadal.wl.keycodes[KEY_SPACE]      = AMW_KEY_SPACE;
    hadal.wl.keycodes[KEY_MINUS]      = AMW_KEY_MINUS;
    hadal.wl.keycodes[KEY_EQUAL]      = AMW_KEY_EQUAL;
    hadal.wl.keycodes[KEY_Q]          = AMW_KEY_Q;
    hadal.wl.keycodes[KEY_W]          = AMW_KEY_W;
    hadal.wl.keycodes[KEY_E]          = AMW_KEY_E;
    hadal.wl.keycodes[KEY_R]          = AMW_KEY_R;
    hadal.wl.keycodes[KEY_T]          = AMW_KEY_T;
    hadal.wl.keycodes[KEY_Y]          = AMW_KEY_Y;
    hadal.wl.keycodes[KEY_U]          = AMW_KEY_U;
    hadal.wl.keycodes[KEY_I]          = AMW_KEY_I;
    hadal.wl.keycodes[KEY_O]          = AMW_KEY_O;
    hadal.wl.keycodes[KEY_P]          = AMW_KEY_P;
    hadal.wl.keycodes[KEY_LEFTBRACE]  = AMW_KEY_LEFT_BRACKET;
    hadal.wl.keycodes[KEY_RIGHTBRACE] = AMW_KEY_RIGHT_BRACKET;
    hadal.wl.keycodes[KEY_A]          = AMW_KEY_A;
    hadal.wl.keycodes[KEY_S]          = AMW_KEY_S;
    hadal.wl.keycodes[KEY_D]          = AMW_KEY_D;
    hadal.wl.keycodes[KEY_F]          = AMW_KEY_F;
    hadal.wl.keycodes[KEY_G]          = AMW_KEY_G;
    hadal.wl.keycodes[KEY_H]          = AMW_KEY_H;
    hadal.wl.keycodes[KEY_J]          = AMW_KEY_J;
    hadal.wl.keycodes[KEY_K]          = AMW_KEY_K;
    hadal.wl.keycodes[KEY_L]          = AMW_KEY_L;
    hadal.wl.keycodes[KEY_SEMICOLON]  = AMW_KEY_SEMICOLON;
    hadal.wl.keycodes[KEY_APOSTROPHE] = AMW_KEY_APOSTROPHE;
    hadal.wl.keycodes[KEY_Z]          = AMW_KEY_Z;
    hadal.wl.keycodes[KEY_X]          = AMW_KEY_X;
    hadal.wl.keycodes[KEY_C]          = AMW_KEY_C;
    hadal.wl.keycodes[KEY_V]          = AMW_KEY_V;
    hadal.wl.keycodes[KEY_B]          = AMW_KEY_B;
    hadal.wl.keycodes[KEY_N]          = AMW_KEY_N;
    hadal.wl.keycodes[KEY_M]          = AMW_KEY_M;
    hadal.wl.keycodes[KEY_COMMA]      = AMW_KEY_COMMA;
    hadal.wl.keycodes[KEY_DOT]        = AMW_KEY_PERIOD;
    hadal.wl.keycodes[KEY_SLASH]      = AMW_KEY_SLASH;
    hadal.wl.keycodes[KEY_BACKSLASH]  = AMW_KEY_BACKSLASH;
    hadal.wl.keycodes[KEY_ESC]        = AMW_KEY_ESCAPE;
    hadal.wl.keycodes[KEY_TAB]        = AMW_KEY_TAB;
    hadal.wl.keycodes[KEY_LEFTSHIFT]  = AMW_KEY_LEFT_SHIFT;
    hadal.wl.keycodes[KEY_RIGHTSHIFT] = AMW_KEY_RIGHT_SHIFT;
    hadal.wl.keycodes[KEY_LEFTCTRL]   = AMW_KEY_LEFT_CONTROL;
    hadal.wl.keycodes[KEY_RIGHTCTRL]  = AMW_KEY_RIGHT_CONTROL;
    hadal.wl.keycodes[KEY_LEFTALT]    = AMW_KEY_LEFT_ALT;
    hadal.wl.keycodes[KEY_RIGHTALT]   = AMW_KEY_RIGHT_ALT;
    hadal.wl.keycodes[KEY_LEFTMETA]   = AMW_KEY_LEFT_SUPER;
    hadal.wl.keycodes[KEY_RIGHTMETA]  = AMW_KEY_RIGHT_SUPER;
    hadal.wl.keycodes[KEY_COMPOSE]    = AMW_KEY_MENU;
    hadal.wl.keycodes[KEY_NUMLOCK]    = AMW_KEY_NUM_LOCK;
    hadal.wl.keycodes[KEY_CAPSLOCK]   = AMW_KEY_CAPS_LOCK;
    hadal.wl.keycodes[KEY_PRINT]      = AMW_KEY_PRINT_SCREEN;
    hadal.wl.keycodes[KEY_SCROLLLOCK] = AMW_KEY_SCROLL_LOCK;
    hadal.wl.keycodes[KEY_PAUSE]      = AMW_KEY_PAUSE;
    hadal.wl.keycodes[KEY_DELETE]     = AMW_KEY_DELETE;
    hadal.wl.keycodes[KEY_BACKSPACE]  = AMW_KEY_BACKSPACE;
    hadal.wl.keycodes[KEY_ENTER]      = AMW_KEY_ENTER;
    hadal.wl.keycodes[KEY_HOME]       = AMW_KEY_HOME;
    hadal.wl.keycodes[KEY_END]        = AMW_KEY_END;
    hadal.wl.keycodes[KEY_PAGEUP]     = AMW_KEY_PAGE_UP;
    hadal.wl.keycodes[KEY_PAGEDOWN]   = AMW_KEY_PAGE_DOWN;
    hadal.wl.keycodes[KEY_INSERT]     = AMW_KEY_INSERT;
    hadal.wl.keycodes[KEY_LEFT]       = AMW_KEY_LEFT;
    hadal.wl.keycodes[KEY_RIGHT]      = AMW_KEY_RIGHT;
    hadal.wl.keycodes[KEY_DOWN]       = AMW_KEY_DOWN;
    hadal.wl.keycodes[KEY_UP]         = AMW_KEY_UP;
    hadal.wl.keycodes[KEY_F1]         = AMW_KEY_F1;
    hadal.wl.keycodes[KEY_F2]         = AMW_KEY_F2;
    hadal.wl.keycodes[KEY_F3]         = AMW_KEY_F3;
    hadal.wl.keycodes[KEY_F4]         = AMW_KEY_F4;
    hadal.wl.keycodes[KEY_F5]         = AMW_KEY_F5;
    hadal.wl.keycodes[KEY_F6]         = AMW_KEY_F6;
    hadal.wl.keycodes[KEY_F7]         = AMW_KEY_F7;
    hadal.wl.keycodes[KEY_F8]         = AMW_KEY_F8;
    hadal.wl.keycodes[KEY_F9]         = AMW_KEY_F9;
    hadal.wl.keycodes[KEY_F10]        = AMW_KEY_F10;
    hadal.wl.keycodes[KEY_F11]        = AMW_KEY_F11;
    hadal.wl.keycodes[KEY_F12]        = AMW_KEY_F12;
    hadal.wl.keycodes[KEY_F13]        = AMW_KEY_F13;
    hadal.wl.keycodes[KEY_F14]        = AMW_KEY_F14;
    hadal.wl.keycodes[KEY_F15]        = AMW_KEY_F15;
    hadal.wl.keycodes[KEY_F16]        = AMW_KEY_F16;
    hadal.wl.keycodes[KEY_F17]        = AMW_KEY_F17;
    hadal.wl.keycodes[KEY_F18]        = AMW_KEY_F18;
    hadal.wl.keycodes[KEY_F19]        = AMW_KEY_F19;
    hadal.wl.keycodes[KEY_F20]        = AMW_KEY_F20;
    hadal.wl.keycodes[KEY_F21]        = AMW_KEY_F21;
    hadal.wl.keycodes[KEY_F22]        = AMW_KEY_F22;
    hadal.wl.keycodes[KEY_F23]        = AMW_KEY_F23;
    hadal.wl.keycodes[KEY_F24]        = AMW_KEY_F24;
    hadal.wl.keycodes[KEY_KPSLASH]    = AMW_KEY_KP_DIVIDE;
    hadal.wl.keycodes[KEY_KPASTERISK] = AMW_KEY_KP_MULTIPLY;
    hadal.wl.keycodes[KEY_KPMINUS]    = AMW_KEY_KP_SUBTRACT;
    hadal.wl.keycodes[KEY_KPPLUS]     = AMW_KEY_KP_ADD;
    hadal.wl.keycodes[KEY_KP0]        = AMW_KEY_KP_0;
    hadal.wl.keycodes[KEY_KP1]        = AMW_KEY_KP_1;
    hadal.wl.keycodes[KEY_KP2]        = AMW_KEY_KP_2;
    hadal.wl.keycodes[KEY_KP3]        = AMW_KEY_KP_3;
    hadal.wl.keycodes[KEY_KP4]        = AMW_KEY_KP_4;
    hadal.wl.keycodes[KEY_KP5]        = AMW_KEY_KP_5;
    hadal.wl.keycodes[KEY_KP6]        = AMW_KEY_KP_6;
    hadal.wl.keycodes[KEY_KP7]        = AMW_KEY_KP_7;
    hadal.wl.keycodes[KEY_KP8]        = AMW_KEY_KP_8;
    hadal.wl.keycodes[KEY_KP9]        = AMW_KEY_KP_9;
    hadal.wl.keycodes[KEY_KPDOT]      = AMW_KEY_KP_DECIMAL;
    hadal.wl.keycodes[KEY_KPEQUAL]    = AMW_KEY_KP_EQUAL;
    hadal.wl.keycodes[KEY_KPENTER]    = AMW_KEY_KP_ENTER;
    hadal.wl.keycodes[KEY_102ND]      = AMW_KEY_WORLD_2;

    for (int32_t scancode = 0; scancode < 256; scancode++) {
        if (hadal.wl.keycodes[scancode] > 0)
            hadal.wl.scancodes[hadal.wl.keycodes[scancode]] = scancode;
    }
}

bool hadal_wayland_connect(void)
{
    amw_log_verbose("Trying to connect to a Wayland display...");

    const hadal_api_t wayland = {
        .id = AMW_HADAL_PLATFORM_WAYLAND,
        .init = hadal_wayland_init,
        .terminate = hadal_wayland_terminate,
        .free_output = hadal_wayland_free_output,
        .video_modes = hadal_wayland_video_modes,
        .video_mode = hadal_wayland_video_mode,
        .get_output_position = hadal_wayland_get_output_position,
        .get_output_content_scale = hadal_wayland_get_output_content_scale,
        .get_output_workarea = hadal_wayland_get_output_workarea,
        .set_clipboard_string = hadal_wayland_set_clipboard_string,
        .get_clipboard_string = hadal_wayland_get_clipboard_string,
        .create_window = hadal_wayland_create_window,
        .destroy_window = hadal_wayland_destroy_window,
        .show_window = hadal_wayland_show_window,
        .hide_window = hadal_wayland_hide_window,
        .retitle_window = hadal_wayland_retitle_window,
        .resize_window = hadal_wayland_resize_window,
        .window_size = hadal_wayland_window_size,
        .framebuffer_size = hadal_wayland_framebuffer_size,
        .content_scale = hadal_wayland_content_scale,
#ifdef AMW_NATIVE_VULKAN
        .physical_device_presentation_support = hadal_wayland_physical_device_presentation_support,
        .create_surface = hadal_wayland_create_surface,
#endif /* AMW_NATIVE_VULKAN */
    };

    if (!hadal_debug_check_api_uptodate(&wayland)) {
        amw_log_debug("Internal API for Wayland is incomplete.");
        return AMW_FALSE;
    }

    struct wl_display *display = wl_display_connect(NULL);
    if (!display) {
        amw_log_debug("Can't connect to Wayland display.");
        return AMW_FALSE;
    }

    hadal.wl.display = display;
    hadal.api = wayland;

    amw_log_verbose("Connected to a Wayland display!");
    return AMW_TRUE;
}

int32_t hadal_wayland_init(void)
{
    amw_log_verbose("Initializing Wayland display backend");
    hadal.wl.key_repeat_timerfd = -1;
    hadal.wl.cursor_timerfd = -1;
    hadal.wl.tag = AMW_VERSIONSTR;

    hadal.wl.registry = wl_display_get_registry(hadal.wl.display);
    wl_registry_add_listener(hadal.wl.registry, &registry_listener, NULL);

    create_key_tables();
    hadal.wl.xkb.context = xkb_context_new(0);
    if (!hadal.wl.xkb.context) {
        amw_log_error("Wayland failed to initialize xkb-context");
        return AMW_FALSE;
    }
    
    /* sync to get all registry objects */
    wl_display_roundtrip(hadal.wl.display);

    /* sync to get all initial output events */
    wl_display_roundtrip(hadal.wl.display);

    if (!hadal.wl.shell) {
        amw_log_error("Wayland compositor is missing xdg-wm-base protocol support.");
        return -1; /* TODO error result code */
    }

    if (!hadal.wl.shm) {
        amw_log_error("Wayland compositor is missing wl-shm.");
        return -1; /* TODO error result code */
    }

    if (hadal.wl.seat && hadal.wl.data_device_manager) {
        hadal.wl.data_device = wl_data_device_manager_get_data_device(hadal.wl.data_device_manager, hadal.wl.seat);
        hadal_wayland_add_data_device_listener(hadal.wl.data_device);
    }
    return 0;
}

void hadal_wayland_terminate(void)
{
    if (hadal.wl.xkb.compose_state)
        xkb_compose_state_unref(hadal.wl.xkb.compose_state);
    if (hadal.wl.xkb.keymap)
        xkb_keymap_unref(hadal.wl.xkb.keymap);
    if (hadal.wl.xkb.state)
        xkb_state_unref(hadal.wl.xkb.state);
    if (hadal.wl.xkb.context)
        xkb_context_unref(hadal.wl.xkb.context);

    for (uint32_t i = 0; i < hadal.wl.offer_count; i++)
        wl_data_offer_destroy(hadal.wl.offers[i].offer);

    amw_free(hadal.wl.offers);

    if (hadal.wl.subcompositor)
        wl_subcompositor_destroy(hadal.wl.subcompositor);
    if (hadal.wl.compositor)
        wl_compositor_destroy(hadal.wl.compositor);
    if (hadal.wl.shm)
        wl_shm_destroy(hadal.wl.shm);
    if (hadal.wl.viewporter)
        wp_viewporter_destroy(hadal.wl.viewporter);
    if (hadal.wl.decoration_manager)
        zxdg_decoration_manager_v1_destroy(hadal.wl.decoration_manager);
    if (hadal.wl.shell)
        xdg_wm_base_destroy(hadal.wl.shell);
    if (hadal.wl.selection_offer)
        wl_data_offer_destroy(hadal.wl.selection_offer);
    if (hadal.wl.drag_offer)
        wl_data_offer_destroy(hadal.wl.drag_offer);
    if (hadal.wl.selection_source)
        wl_data_source_destroy(hadal.wl.selection_source);
    if (hadal.wl.data_device)
        wl_data_device_destroy(hadal.wl.data_device);
    if (hadal.wl.data_device_manager)
        wl_data_device_manager_destroy(hadal.wl.data_device_manager);
    if (hadal.wl.pointer)
        wl_pointer_destroy(hadal.wl.pointer);
    if (hadal.wl.keyboard)
        wl_keyboard_destroy(hadal.wl.keyboard);
    if (hadal.wl.seat)
        wl_seat_destroy(hadal.wl.seat);
    if (hadal.wl.relative_pointer_manager)
        zwp_relative_pointer_manager_v1_destroy(hadal.wl.relative_pointer_manager);
    if (hadal.wl.pointer_constraints)
        zwp_pointer_constraints_v1_destroy(hadal.wl.pointer_constraints);
    if (hadal.wl.idle_inhibit_manager)
        zwp_idle_inhibit_manager_v1_destroy(hadal.wl.idle_inhibit_manager);
    if (hadal.wl.activation_manager)
        xdg_activation_v1_destroy(hadal.wl.activation_manager);
    if (hadal.wl.fractional_scale_manager)
        wp_fractional_scale_manager_v1_destroy(hadal.wl.fractional_scale_manager);
    if (hadal.wl.registry)
        wl_registry_destroy(hadal.wl.registry);
    if (hadal.wl.display) {
        wl_display_flush(hadal.wl.display);
        wl_display_disconnect(hadal.wl.display);
    }

    if (hadal.wl.key_repeat_timerfd >= 0)
        close(hadal.wl.key_repeat_timerfd);
    if (hadal.wl.cursor_timerfd >= 0)
        close(hadal.wl.cursor_timerfd);

    amw_arena_free(&hadal.clipboard_arena);
}

