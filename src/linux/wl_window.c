#include "../hadopelagic.h"

static void set_content_area_opaque(amw_window_t *window)
{
    if (!hadal.wl.compositor)
        amw_log_error(" co do kurwy");

    struct wl_region *region;
    region = wl_compositor_create_region(hadal.wl.compositor);

    if (!region)
        return;

    wl_region_add(region, 0, 0, window->wl.width, window->wl.height);
    wl_surface_set_opaque_region(window->wl.surface, region);
    wl_region_destroy(region);
}

static void resize_framebuffer(amw_window_t *window)
{
    if (window->wl.fractional_scale) {
        window->wl.fb_width = (window->wl.width * window->wl.scaling_numerator) / 120;
        window->wl.fb_height = (window->wl.height * window->wl.scaling_numerator) / 120;
    } else {
        window->wl.fb_width = window->wl.width * window->wl.buffer_scale;
        window->wl.fb_height = window->wl.height * window->wl.buffer_scale;
    }

    if (!window->transparent)
        set_content_area_opaque(window);
    hadal_input_framebuffer_resized_callback(window, window->wl.fb_width, window->wl.fb_height);
}

static bool resize_window(amw_window_t *window, int32_t width, int32_t height)
{
    width = amw_max(width, 1);
    height = amw_max(height, 1);

    if (width == window->wl.width && height == window->wl.height)
        return AMW_FALSE;

    window->wl.width = width;
    window->wl.height = height;

    resize_framebuffer(window);

    if (window->wl.scaling_viewport)
        wp_viewport_set_destination(window->wl.scaling_viewport, window->wl.width, window->wl.height);

    return AMW_TRUE;
}

static bool flush_display(void)
{
    while (wl_display_flush(hadal.wl.display) == -1) {
        if (errno != EAGAIN)
            return AMW_FALSE;
        struct pollfd fd = { wl_display_get_fd(hadal.wl.display), POLLOUT, 0 };
        while (poll(&fd, 1, -1) == -1) {
            if (errno != EINTR && errno != EAGAIN)
                return AMW_FALSE;
        }
    }
    return AMW_TRUE;
}

static char *read_data_offer_as_string(struct wl_data_offer *offer, const char *mimetype)
{
    int32_t fds[2];

    if (pipe2(fds, O_CLOEXEC) == -1) {
        amw_log_error("Wayland failed to create pipe for data offer: %s", strerror(errno));
        return NULL;
    }
    wl_data_offer_receive(offer, mimetype, fds[1]);
    flush_display();
    close(fds[1]);

    char   *str = NULL;
    size_t  size = 0;
    size_t  len = 0;

    for (;;) {
        const size_t read_size = 4096;
        const size_t required_size = len + read_size + 1;
        if (required_size > size) {
            char *longer = amw_realloc(str, required_size);
            if (!longer) {
                amw_log_error("Out of memory");
                close(fds[0]);
                return NULL;
            }
            str = longer;
            size = required_size;
        }
        const ssize_t res = read(fds[0], str + len, read_size);
        if (res == 0) {
            break;
        } else if (res == -1) {
            if (errno == EINTR)
                continue;
            amw_log_error("Wayland failed to read from data offer pipe: %s", strerror(errno));
            close(fds[0]);
            return NULL;
        }
        len += res;
    }
    close(fds[0]);
    str[len] = '\0';
    return str;
}

static void surface_handle_enter(void *data,
                                 struct wl_surface *surface,
                                 struct wl_output *wl_output)
{
    if (wl_proxy_get_tag((struct wl_proxy *)wl_output) != &hadal.wl.tag)
        return;

    /* unused */
    (void)surface;

    amw_window_t *window = data;
    amw_output_t *output = wl_output_get_user_data(wl_output);
    if (!window || !output)
        return;

    if (window->wl.output_scale_count + 1 > window->wl.output_scale_size) {
        window->wl.output_scale_size++;
        window->wl.output_scales = amw_realloc(window->wl.output_scales, 
                                               window->wl.output_scale_size * sizeof(amw_wayland_scale_t));
    }
    window->wl.output_scale_count++;
    window->wl.output_scales[window->wl.output_scale_count - 1] = (amw_wayland_scale_t) { wl_output, output->wl.scale };
    hadal_wayland_update_buffer_scale_from_outputs(window);
}

static void surface_handle_leave(void *data,
                                 struct wl_surface *surface,
                                 struct wl_output *wl_output)
{
    if (wl_proxy_get_tag((struct wl_proxy *)wl_output) != &hadal.wl.tag)
        return;

    /* unused */
    (void)surface;

    amw_window_t *window = data;

    for (size_t i = 0; i < window->wl.output_scale_count; i++) {
        if (window->wl.output_scales[i].output == wl_output) {
            window->wl.output_scales[i] = window->wl.output_scales[window->wl.output_scale_count - 1];
            window->wl.output_scale_count--;
            break;
        }
    }
    hadal_wayland_update_buffer_scale_from_outputs(window);
}

static const struct wl_surface_listener surface_listener = {
    .enter = surface_handle_enter,
    .leave = surface_handle_leave,
    .preferred_buffer_scale = NULL,
    .preferred_buffer_transform = NULL,
};

static void set_idle_inhibitor(amw_window_t *window, bool enable)
{
    if (enable && !window->wl.idle_inhibitor && hadal.wl.idle_inhibit_manager) {
        window->wl.idle_inhibitor = zwp_idle_inhibit_manager_v1_create_inhibitor(
                hadal.wl.idle_inhibit_manager, window->wl.surface);
        if (!window->wl.idle_inhibitor)
            amw_log_error("Wayland failed to create idle inhibitor");
    } else if (!enable && window->wl.idle_inhibitor) {
        zwp_idle_inhibitor_v1_destroy(window->wl.idle_inhibitor);
        window->wl.idle_inhibitor = NULL;
    }
}

static void acquire_output(amw_window_t *window)
{
    if (window->wl.xdg.toplevel)
        xdg_toplevel_set_fullscreen(window->wl.xdg.toplevel, window->output->wl.output);
    set_idle_inhibitor(window, AMW_TRUE);
}

static void release_output(amw_window_t *window) 
{
    if (window->wl.xdg.toplevel)
        xdg_toplevel_unset_fullscreen(window->wl.xdg.toplevel);
    set_idle_inhibitor(window, AMW_FALSE);
}

static void fractional_scale_handle_preferred_scale(void *data,
                                             struct wp_fractional_scale_v1 *fractional_scale,
                                             uint32_t numerator)
{
    /* unused */
    (void)fractional_scale;

    amw_window_t *window = data;
    window->wl.scaling_numerator = numerator;
    resize_framebuffer(window);

    // FIXME
    //if (window->visible)
    //  hadal_input_window_damage(window);
}

static const struct wp_fractional_scale_v1_listener fractional_scale_listener = {
    .preferred_scale = fractional_scale_handle_preferred_scale,
};

static void xdg_toplevel_handle_configure(void* data,
                                          struct xdg_toplevel* toplevel,
                                          int32_t width,
                                          int32_t height,
                                          struct wl_array* states)
{
    /* unused */
    (void)toplevel;

    amw_window_t *window = data;
    uint32_t     *state;

    window->wl.pending.activated = AMW_FALSE;
    window->wl.pending.maximized = AMW_FALSE;
    window->wl.pending.fullscreen = AMW_FALSE;

    wl_array_for_each(state, states) {
        switch (*state) {
        case XDG_TOPLEVEL_STATE_MAXIMIZED:
            window->wl.pending.maximized = AMW_TRUE;
            break;
        case XDG_TOPLEVEL_STATE_FULLSCREEN:
            window->wl.pending.fullscreen = AMW_TRUE;
            break;
        case XDG_TOPLEVEL_STATE_RESIZING:
            window->wl.pending.resizing = AMW_TRUE;
            break;
        case XDG_TOPLEVEL_STATE_ACTIVATED:
            window->wl.pending.activated = AMW_TRUE;
            break;
        }
    }

    if (width && height) {
        window->wl.pending.width  = width;
        window->wl.pending.height = height;
    } else {
        window->wl.pending.width  = window->wl.width;
        window->wl.pending.height = window->wl.height;
    }
}

static void xdg_toplevel_handle_close(void *data, struct xdg_toplevel *toplevel)
{
    /* unused */
    (void)toplevel;

    amw_window_t *window = data;
    window->should_close = AMW_TRUE;
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_handle_configure,
    .close     = xdg_toplevel_handle_close,
};

static void xdg_surface_handle_configure(void *data, struct xdg_surface *surface, uint32_t serial)
{
    amw_window_t *window = data;

    xdg_surface_ack_configure(surface, serial);
    if (window->activated != window->wl.pending.activated) {
        window->activated = window->wl.pending.activated;
        if (!window->activated) {
            if (window->output && window->auto_iconify)
                xdg_toplevel_set_minimized(window->wl.xdg.toplevel);
        }
    }
    if (window->maximized != window->wl.pending.maximized) {
        window->maximized = window->wl.pending.maximized;
        // TODO window maximize callback
    }
    window->fullscreen = window->wl.pending.fullscreen;

    int32_t width  = window->wl.pending.width;
    int32_t height = window->wl.pending.height;

    if (!window->maximized && !window->fullscreen) {
        if (window->numer != HADAL_DONT_CARE && window->denom != HADAL_DONT_CARE) {
            const float aspect_ratio = (float)width / (float)height;
            const float target_ratio = (float)window->numer / (float)window->denom;
            if (aspect_ratio < target_ratio)
                height = width / target_ratio;
            else if (aspect_ratio > target_ratio)
                width = height * target_ratio;
        }
    }

    if (resize_window(window, width, height)) {
        // FIXME window resized callback
        // FIXME
        //if (window->visible)
        //  hadal_input_window_damage(window);
    }
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_handle_configure,
};

static void update_xdg_size_limits(amw_window_t *window)
{
    int32_t minwidth, minheight, maxwidth, maxheight;

    if (window->resizable) {
        if (window->minwidth == HADAL_DONT_CARE || window->minheight == HADAL_DONT_CARE) {
            minwidth = minheight = 0;
        } else {
            minwidth  = window->minwidth;
            minheight = window->minheight;
        }

        if (window->maxwidth == HADAL_DONT_CARE || window->maxheight == HADAL_DONT_CARE) {
            maxwidth = maxheight = 0;
        } else {
            maxwidth  = window->maxwidth;
            maxheight = window->maxheight;
        }
    } else {
        minwidth  = maxwidth  = window->wl.width;
        minheight = maxheight = window->wl.height;
    }

    xdg_toplevel_set_min_size(window->wl.xdg.toplevel, minwidth, minheight);
    xdg_toplevel_set_max_size(window->wl.xdg.toplevel, maxwidth, maxheight);
}

static bool create_xdg_shell_objects(amw_window_t *window)
{
    amw_log_debug("Creating xdg shell objects");
    window->wl.xdg.surface = xdg_wm_base_get_xdg_surface(hadal.wl.shell, window->wl.surface);
    if (!window->wl.xdg.surface) {
        amw_log_error("Wayland failed to create xdg-surface for window");
        return AMW_FALSE;
    }
    xdg_surface_add_listener(window->wl.xdg.surface, &xdg_surface_listener, window);

    window->wl.xdg.toplevel = xdg_surface_get_toplevel(window->wl.xdg.surface);
    if (!window->wl.xdg.toplevel) {
        amw_log_error("Wayland failed to create xdg-toplevel for window");
        return AMW_FALSE;
    }
    xdg_toplevel_add_listener(window->wl.xdg.toplevel, &xdg_toplevel_listener, window);

    if (window->wl.app_id)
        xdg_toplevel_set_app_id(window->wl.xdg.toplevel, window->wl.app_id);
    xdg_toplevel_set_title(window->wl.xdg.toplevel, window->title);

    if (window->output) {
        xdg_toplevel_set_fullscreen(window->wl.xdg.toplevel, window->output->wl.output);
        set_idle_inhibitor(window, AMW_TRUE);
    } else {
        if (window->maximized)
            xdg_toplevel_set_maximized(window->wl.xdg.toplevel);
        set_idle_inhibitor(window, AMW_FALSE);
    }

    update_xdg_size_limits(window);
    wl_surface_commit(window->wl.surface);
    wl_display_roundtrip(hadal.wl.display);
    return AMW_TRUE;
}

static void destroy_shell_objects(amw_window_t *window)
{
    if (window->wl.xdg.decoration)
        zxdg_toplevel_decoration_v1_destroy(window->wl.xdg.decoration);
    if (window->wl.xdg.toplevel)
        xdg_toplevel_destroy(window->wl.xdg.toplevel);
    if (window->wl.xdg.surface)
        xdg_surface_destroy(window->wl.xdg.surface);

    window->wl.xdg.decoration = NULL;
    window->wl.xdg.decoration_mode = 0;
    window->wl.xdg.toplevel   = NULL;
    window->wl.xdg.surface    = NULL;
}

static bool create_native_surface(amw_window_t *window, int32_t width, int32_t height)
{
    window->wl.surface = wl_compositor_create_surface(hadal.wl.compositor);
    if (!window->wl.surface) {
        amw_log_error("Wayland failed to create a window surface");
        return AMW_FALSE;
    }

    wl_proxy_set_tag((struct wl_proxy *)window->wl.surface, &hadal.wl.tag);
    wl_surface_add_listener(window->wl.surface, &surface_listener, window);

    window->wl.width = width;
    window->wl.height = height;
    window->wl.fb_width = width;
    window->wl.fb_height = height;
    window->wl.buffer_scale = 1;
    window->wl.scaling_numerator = 120;

    if (!window->transparent) {
        set_content_area_opaque(window);
    }

    if (hadal.wl.fractional_scale_manager) {
        if (window->scale_framebuffer)
            window->wl.scaling_viewport = wp_viewporter_get_viewport(hadal.wl.viewporter, window->wl.surface);
        wp_viewport_set_destination(window->wl.scaling_viewport, window->wl.width, window->wl.height);

        window->wl.fractional_scale = wp_fractional_scale_manager_v1_get_fractional_scale(
                hadal.wl.fractional_scale_manager, window->wl.surface);
        wp_fractional_scale_v1_add_listener(window->wl.fractional_scale, &fractional_scale_listener, window);
    }
    return AMW_TRUE;
}

static int32_t translate_key(uint32_t scancode)
{
    if (scancode < amw_arraysize(hadal.wl.keycodes))
        return hadal.wl.keycodes[scancode];
    return AMW_KEY_INVALID;
}

static xkb_keysym_t compose_symbol(xkb_keysym_t sym)
{
    if (sym == XKB_KEY_NoSymbol || !hadal.wl.xkb.compose_state)
        return sym;
    if (xkb_compose_state_feed(hadal.wl.xkb.compose_state, sym) != XKB_COMPOSE_FEED_ACCEPTED)
        return sym;
    switch (xkb_compose_state_get_status(hadal.wl.xkb.compose_state)) {
    case XKB_COMPOSE_COMPOSED:
        return xkb_compose_state_get_one_sym(hadal.wl.xkb.compose_state);
    case XKB_COMPOSE_COMPOSING:
    case XKB_COMPOSE_CANCELLED:
        return XKB_KEY_NoSymbol;
    case XKB_COMPOSE_NOTHING:
    default: 
        return sym;
    }
}

static void input_text(amw_window_t *window, uint32_t scancode)
{
    (void)window;
    (void)scancode;
    /* TODO unicode */
}

static void handle_events(double *timeout)
{
    bool event = AMW_FALSE;
    enum { DISPLAY_FD, KEYREPEAT_FD, CURSOR_FD };
    struct pollfd fds[] = {
        [DISPLAY_FD]   = { wl_display_get_fd(hadal.wl.display), POLLIN, 0 },
        [KEYREPEAT_FD] = { hadal.wl.key_repeat_timerfd, POLLIN, 0 },
        [CURSOR_FD]    = { hadal.wl.cursor_timerfd, POLLIN, 0 },
    };

    while (!event) {
        while (wl_display_prepare_read(hadal.wl.display) != 0) {
            if (wl_display_dispatch_pending(hadal.wl.display) > 0)
                return;
        }

        /* if an error other than EAGAIN happens, we have likely been disconnected
         * from the wayland session... */
        if (!flush_display()) {
            wl_display_cancel_read(hadal.wl.display);

            amw_window_t *window = hadal.window_list;
            while (window) {
                window->should_close = AMW_TRUE;
                window = window->next;
            }
            return;
        }

        if (!hadal_poll_posix(fds, amw_arraysize(fds), timeout)) {
            wl_display_cancel_read(hadal.wl.display);
            return;
        }

        if (fds[DISPLAY_FD].revents & POLLIN) {
            wl_display_read_events(hadal.wl.display);
            if (wl_display_dispatch_pending(hadal.wl.display) > 0)
                event = AMW_TRUE;
        } else {
            wl_display_cancel_read(hadal.wl.display);
        }

        if (fds[KEYREPEAT_FD].revents & POLLIN) {
            uint64_t repeats;

            if (read(hadal.wl.key_repeat_timerfd, &repeats, sizeof(repeats)) == 8) {
                for (uint64_t i = 0; i < repeats; i++) {
                    hadal_input_key_callback(hadal.wl.keyboard_focus,
                                             translate_key(hadal.wl.key_repeat_scancode),
                                             hadal.wl.key_repeat_scancode,
                                             AMW_INPUT_PRESS,
                                             hadal.wl.xkb.modifiers);
                    input_text(hadal.wl.keyboard_focus, hadal.wl.key_repeat_scancode);
                }
                event = AMW_TRUE;
            }
        }

        if (fds[CURSOR_FD].revents & POLLIN) {
            uint64_t repeats;

            if (read(hadal.wl.cursor_timerfd, &repeats, sizeof(repeats)) == 8) {
                // increment cursor image
                amw_log_debug("stuck ??");
            }
        }
    }
}

static void pointer_handle_enter(void *data,
                                 struct wl_pointer *pointer,
                                 uint32_t serial,
                                 struct wl_surface *surface,
                                 wl_fixed_t sx,
                                 wl_fixed_t sy)
{
    /* unused */
    (void)data;
    (void)pointer;
    (void)sx;
    (void)sy;

    /* in case we just destroyed the surface */
    if (!surface)
        return;

    if (wl_proxy_get_tag((struct wl_proxy *)surface) != &hadal.wl.tag)
        return;

    amw_window_t *window = wl_surface_get_user_data(surface);

    hadal.wl.serial = serial;
    hadal.wl.pointer_enter_serial = serial;
    hadal.wl.pointer_focus = window;

    if (surface == window->wl.surface) {
        window->hovered = AMW_TRUE;
        /* TODO handle cursor enter */
    } else {
        /* TODO decorations */
    }
}

static void pointer_handle_leave(void *data,
                                 struct wl_pointer *pointer,
                                 uint32_t serial,
                                 struct wl_surface *surface)
{
    /* unused */
    (void)data;
    (void)pointer;

    if (!surface)
        return;

    if (wl_proxy_get_tag((struct wl_proxy *)surface) != &hadal.wl.tag)
        return;

    amw_window_t *window = hadal.wl.pointer_focus;
    if (!window)
        return;

    hadal.wl.serial = serial;
    hadal.wl.pointer_focus = NULL;

    if (window->hovered) {
        window->hovered = AMW_FALSE;
        /* TODO handle cursor leave */
    } else {
        /* TODO decorations */
    }
}

static void pointer_handle_motion(void *data,
                                  struct wl_pointer *pointer,
                                  uint32_t time,
                                  wl_fixed_t sx,
                                  wl_fixed_t sy)
{
    /* unused */
    (void)data;
    (void)time;
    (void)pointer;

    amw_window_t *window = hadal.wl.pointer_focus;
    if (!window)
        return;

    if (window->cursor_mode == HADAL_CURSOR_DISABLED)
        return;

    const double xpos = wl_fixed_to_double(sx);
    const double ypos = wl_fixed_to_double(sy);
    window->wl.cursor_xpos = xpos;
    window->wl.cursor_ypos = ypos;

    if (window->hovered) {
        // TODO cursor position callback
        return;
    }
}

static void pointer_handle_button(void *data,
                                  struct wl_pointer *pointer,
                                  uint32_t serial,
                                  uint32_t time,
                                  uint32_t button,
                                  uint32_t state)
{
    /* unused */
    (void)data;
    (void)pointer;
    (void)time;
    (void)button;
    (void)state;

    amw_window_t *window = hadal.wl.pointer_focus;
    if (!window)
        return;

    if (window->hovered) {
        hadal.wl.serial = serial;
        // TODO mouse click callback
        return;
    }
}

static void pointer_handle_axis(void *data,
                                struct wl_pointer *pointer,
                                uint32_t time,
                                uint32_t axis,
                                wl_fixed_t value)
{
    /* unused */
    (void)data;
    (void)pointer;
    (void)time;
    (void)value;

    amw_window_t *window = hadal.wl.pointer_focus;
    if (!window)
        return;

    if (axis == WL_POINTER_AXIS_HORIZONTAL_SCROLL) {
        // scroll callback
    } else if (axis == WL_POINTER_AXIS_VERTICAL_SCROLL) {
        // scroll callback
    }
}

static const struct wl_pointer_listener pointer_listener = {
    .enter = pointer_handle_enter,
    .leave = pointer_handle_leave,
    .motion = pointer_handle_motion,
    .button = pointer_handle_button,
    .axis = pointer_handle_axis,
};

static void keyboard_handle_keymap(void *data,
                                   struct wl_keyboard *keyboard,
                                   uint32_t format,
                                   int32_t fd,
                                   uint32_t size)
{
    /* unused */
    (void)data;
    (void)keyboard;

    struct xkb_keymap *keymap;
    struct xkb_state  *state;
    struct xkb_compose_table *compose_table;
    struct xkb_compose_state *compose_state;

    char *mapstr;
    const char *locale;

    if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
        close(fd);
        return;
    }

    mapstr = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
    if (mapstr == MAP_FAILED) {
        close(fd);
        return;
    }
    keymap = xkb_keymap_new_from_string(hadal.wl.xkb.context, mapstr, XKB_KEYMAP_FORMAT_TEXT_V1, 0);
    munmap(mapstr, size);
    close(fd);

    if (!keymap) {
        amw_log_error("Wayland failed to compile keymap");
        return;
    }

    state = xkb_state_new(keymap);
    if (!state) { 
        amw_log_error("Wayland failed to create XKB state");
        xkb_keymap_unref(keymap);
        return;
    }

    locale = getenv("LC_ALL");
    if (!locale)
        locale = getenv("LC_CTYPE");
    if (!locale)
        locale = getenv("LANG");
    if (!locale)
        locale = "C";

    compose_table = xkb_compose_table_new_from_locale(hadal.wl.xkb.context, locale, XKB_COMPOSE_COMPILE_NO_FLAGS);

    if (compose_table) {
        compose_state = xkb_compose_state_new(compose_table, XKB_COMPOSE_STATE_NO_FLAGS);
        xkb_compose_table_unref(compose_table);

        if (compose_state) {
            hadal.wl.xkb.compose_state = compose_state;
        } else {
            amw_log_error("Wayland failed to create XKB compose state");
        }
    } else {
        amw_log_error("Wayland failed to create XKB compose table");
    }

    xkb_keymap_unref(hadal.wl.xkb.keymap);
    xkb_state_unref(hadal.wl.xkb.state);
    hadal.wl.xkb.keymap = keymap;
    hadal.wl.xkb.state = state;

    hadal.wl.xkb.control_index   = xkb_keymap_mod_get_index(hadal.wl.xkb.keymap, "Control");
    hadal.wl.xkb.alt_index       = xkb_keymap_mod_get_index(hadal.wl.xkb.keymap, "Mod1");
    hadal.wl.xkb.shift_index     = xkb_keymap_mod_get_index(hadal.wl.xkb.keymap, "Shift");
    hadal.wl.xkb.super_index     = xkb_keymap_mod_get_index(hadal.wl.xkb.keymap, "Mod4");
    hadal.wl.xkb.caps_lock_index = xkb_keymap_mod_get_index(hadal.wl.xkb.keymap, "Lock");
    hadal.wl.xkb.num_lock_index  = xkb_keymap_mod_get_index(hadal.wl.xkb.keymap, "Mod2");
}

static void keyboard_handle_enter(void *data,
                                  struct wl_keyboard *keyboard,
                                  uint32_t serial,
                                  struct wl_surface *surface,
                                  struct wl_array *keys)
{
    /* unused */
    (void)data;
    (void)keyboard;
    (void)keys;

    if (!surface)
        return;

    if (wl_proxy_get_tag((struct wl_proxy *)surface) != &hadal.wl.tag)
        return;

    amw_window_t *window = wl_surface_get_user_data(surface);
    if (surface != window->wl.surface)
        return;

    hadal.wl.serial = serial;
    hadal.wl.keyboard_focus = window;
    // TODO window focus callback, true
}

static void keyboard_handle_leave(void *data,
                                  struct wl_keyboard *keyboard,
                                  uint32_t serial,
                                  struct wl_surface *surface)
{
    /* unused */
    (void)data;
    (void)keyboard;
    (void)surface;

    amw_window_t *window = hadal.wl.keyboard_focus;
    if (!window)
        return;

    struct itimerspec timer = {0};
    timerfd_settime(hadal.wl.key_repeat_timerfd, 0, &timer, NULL);

    hadal.wl.serial = serial;
    hadal.wl.keyboard_focus = NULL;
    // TODO window focus callback, false
}

static void keyboard_handle_key(void *data,
                                struct wl_keyboard *keyboard,
                                uint32_t serial,
                                uint32_t time,
                                uint32_t scancode,
                                uint32_t state)
{
    /* unused */
    (void)data;
    (void)keyboard;
    (void)time;

    amw_window_t *window = hadal.wl.keyboard_focus;
    if (!window)
        return;

    const int32_t key = translate_key(scancode);
    const int32_t action = state == WL_KEYBOARD_KEY_STATE_PRESSED ? AMW_INPUT_PRESS : AMW_INPUT_RELEASE;

    hadal.wl.serial = serial;
    struct itimerspec timer = {0};

    if (action == AMW_INPUT_PRESS) {
        const xkb_keycode_t keycode = scancode + 8;

        if (xkb_keymap_key_repeats(hadal.wl.xkb.keymap, keycode) && hadal.wl.key_repeat_rate > 0) {
            hadal.wl.key_repeat_scancode = scancode;
            if (hadal.wl.key_repeat_rate > 1) {
                timer.it_interval.tv_nsec = 1000000000 / hadal.wl.key_repeat_rate;
            } else {
                timer.it_interval.tv_sec = 1;
            }
            timer.it_value.tv_sec  = hadal.wl.key_repeat_delay / 1000;
            timer.it_value.tv_nsec = (hadal.wl.key_repeat_delay % 1000) * 1000000;
        }
    }
    timerfd_settime(hadal.wl.key_repeat_timerfd, 0, &timer, NULL);
    hadal_input_key_callback(window, key, scancode, action, hadal.wl.xkb.modifiers);
    if (action == AMW_INPUT_PRESS) {
        input_text(window, scancode);
    }
}

static void keyboard_handle_modifiers(void *data,
                                      struct wl_keyboard *keyboard,
                                      uint32_t serial,
                                      uint32_t mods_depressed,
                                      uint32_t mods_latched,
                                      uint32_t mods_locked,
                                      uint32_t group)
{
    /* unused */
    (void)data;
    (void)keyboard;

    hadal.wl.serial = serial;
    if (!hadal.wl.xkb.keymap)
        return;

    xkb_state_update_mask(hadal.wl.xkb.state, mods_depressed, mods_latched, mods_locked, 0, 0, group);
    hadal.wl.xkb.modifiers = 0;

    struct {
        xkb_mod_index_t index;
        uint32_t        bit;
    } modifiers[] = {
        { hadal.wl.xkb.control_index,   AMW_KEYMOD_CTRL },
        { hadal.wl.xkb.alt_index,       AMW_KEYMOD_ALT },
        { hadal.wl.xkb.shift_index,     AMW_KEYMOD_SHIFT },
        { hadal.wl.xkb.super_index,     AMW_KEYMOD_SUPER },
        { hadal.wl.xkb.caps_lock_index, AMW_KEYMOD_CAPS_LOCK },
        { hadal.wl.xkb.num_lock_index,  AMW_KEYMOD_NUM_LOCK },
    };

    for (size_t i = 0; i < amw_arraysize(modifiers); i++) {
        if (xkb_state_mod_index_is_active(hadal.wl.xkb.state, modifiers[i].index, XKB_STATE_MODS_EFFECTIVE) == 1) {
            hadal.wl.xkb.modifiers |= modifiers[i].bit;
        }
    }
}

static void keyboard_handle_repeat_info(void *data, 
                                        struct wl_keyboard *keyboard, 
                                        int32_t rate, 
                                        int32_t delay)
{
    /* unused */
    (void)data;

    if (keyboard != hadal.wl.keyboard) 
        return;

    hadal.wl.key_repeat_rate = rate;
    hadal.wl.key_repeat_delay = delay;
}

static const struct wl_keyboard_listener keyboard_listener = {
    .keymap      = keyboard_handle_keymap,
    .enter       = keyboard_handle_enter,
    .leave       = keyboard_handle_leave,
    .key         = keyboard_handle_key,
    .modifiers   = keyboard_handle_modifiers,
    .repeat_info = keyboard_handle_repeat_info, 
};

static void seat_handle_capabilities(void *data, 
                                     struct wl_seat *seat, 
                                     enum wl_seat_capability caps)
{
    if ((caps & WL_SEAT_CAPABILITY_POINTER) && !hadal.wl.pointer) {
        hadal.wl.pointer = wl_seat_get_pointer(seat);
        wl_pointer_add_listener(hadal.wl.pointer, &pointer_listener, NULL);
    } else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && hadal.wl.pointer) {
        wl_pointer_destroy(hadal.wl.pointer);
        hadal.wl.pointer = NULL;
    }

    if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !hadal.wl.keyboard) {
        hadal.wl.keyboard = wl_seat_get_keyboard(seat);
        wl_keyboard_add_listener(hadal.wl.keyboard, &keyboard_listener, NULL);
    } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD)) {
        wl_keyboard_destroy(hadal.wl.keyboard);
        hadal.wl.keyboard = NULL;
    }
    // TODO touch devices
}

static void seat_handle_name(void *data, 
                             struct wl_seat *seat, 
                             const char *name)
{
    /* unused */
    (void)data;
    (void)seat;
    (void)name;
}

static const struct wl_seat_listener seat_listener = {
    .capabilities = seat_handle_capabilities,
    .name         = seat_handle_name,
};

static void data_offer_handle_offer(void *data, 
                                    struct wl_data_offer *offer, 
                                    const char *mimetype)
{
    /* unused */
    (void)data;

    for (uint32_t i = 0; i < hadal.wl.offer_count; i++)    {
        if (hadal.wl.offers[i].offer == offer) {
            if (strcmp(mimetype, "text/plain;charset=utf-8") == 0) {
                hadal.wl.offers[i].text_plain_utf8 = AMW_TRUE;
            } else if (strcmp(mimetype, "text/uri-list") == 0) {
                hadal.wl.offers[i].text_uri_list = AMW_TRUE;
            }
            break;
        }
    }
}

static const struct wl_data_offer_listener data_offer_listener = {
    .offer = data_offer_handle_offer,
};

static void data_device_handle_data_offer(void *data, struct wl_data_device *device, struct wl_data_offer *offer)
{
    /* unused */
    (void)data;
    (void)device;

    amw_wayland_offer_t *offers = amw_realloc(hadal.wl.offers, sizeof(amw_wayland_offer_t) * (hadal.wl.offer_count + 1));
    if (!offers) {
        amw_log_error("Out of memory");
        return;
    }

    hadal.wl.offers = offers;
    hadal.wl.offer_count++;

    hadal.wl.offers[hadal.wl.offer_count - 1] = (amw_wayland_offer_t) { offer, 0, 0 };
    wl_data_offer_add_listener(offer, &data_offer_listener, NULL);
}

static void data_device_handle_enter(void *data, 
                                     struct wl_data_device *device,
                                     uint32_t serial,
                                     struct wl_surface *surface,
                                     wl_fixed_t x,
                                     wl_fixed_t y,
                                     struct wl_data_offer *offer)
{
    /* unused */
    (void)data;
    (void)device;
    (void)x;
    (void)y;

    if (hadal.wl.drag_offer) {
        wl_data_offer_destroy(hadal.wl.drag_offer);
        hadal.wl.drag_offer = NULL;
        hadal.wl.drag_focus = NULL;
    }

    uint32_t i;

    for (i = 0; i < hadal.wl.offer_count; i++) {
        if (hadal.wl.offers[i].offer == offer)
            break;
    }

    if (i == hadal.wl.offer_count)
        return;

    if (surface && wl_proxy_get_tag((struct wl_proxy *)surface) == &hadal.wl.tag) {
        amw_window_t *window = wl_surface_get_user_data(surface);
        if (window->wl.surface == surface) {
            if (hadal.wl.offers[i].text_uri_list) {
                hadal.wl.drag_offer = offer;
                hadal.wl.drag_focus = window;
                hadal.wl.drag_serial = serial;
                wl_data_offer_accept(offer, serial, "text/uri-list");
            }
        }
    }

    if (!hadal.wl.drag_offer) {
        wl_data_offer_accept(offer, serial, NULL);
        wl_data_offer_destroy(offer);
    }

    hadal.wl.offers[i] = hadal.wl.offers[hadal.wl.offer_count - 1];
    hadal.wl.offer_count--;
}

static void data_device_handle_leave(void *data, struct wl_data_device *device)
{
    /* unused */
    (void)data;
    (void)device;

    if (hadal.wl.drag_offer) {
        wl_data_offer_destroy(hadal.wl.drag_offer);
        hadal.wl.drag_offer = NULL;
        hadal.wl.drag_focus = NULL;
    }
}

static void data_device_handle_motion(void *data, 
                                      struct wl_data_device *device, 
                                      uint32_t time, 
                                      wl_fixed_t x, 
                                      wl_fixed_t y)
{
    /* unused */
    (void)data;
    (void)device;
    (void)time;
    (void)x;
    (void)y;
}

static void data_device_handle_drop(void *data, struct wl_data_device *device)
{
    /* unused */
    (void)data;
    (void)device;

    if (!hadal.wl.drag_offer)
        return;

    /* TODO use an arena, my stupid ass will have to figure out the string allocations tho... */
    char *str = read_data_offer_as_string(hadal.wl.drag_offer, "text/uri-list");
    if (str) {
        int32_t count;
        char **paths = hadal_parse_uri_list(str, &count);
        if (paths) {
            // TODO drop callback
            for (int32_t i = 0; i < count; i++) {
                amw_free(paths[i]);
            }
            amw_free(paths);
        }
        amw_free(str);
    }
}

static void data_device_handle_selection(void *data, struct wl_data_device *device, struct wl_data_offer *offer)
{
    /* unused */
    (void)data;
    (void)device;

    if (hadal.wl.selection_offer) {
        wl_data_offer_destroy(hadal.wl.selection_offer);
        hadal.wl.selection_offer = NULL;
    }

    for (uint32_t i = 0; i < hadal.wl.offer_count; i++) {
        if (hadal.wl.offers[i].offer == offer) {
            if (hadal.wl.offers[i].text_plain_utf8) {
                hadal.wl.selection_offer = offer;
            } else {
                wl_data_offer_destroy(offer);
            }
            hadal.wl.offers[i] = hadal.wl.offers[hadal.wl.offer_count - 1];
            hadal.wl.offer_count--;
            break;
        }
    }
}

const struct wl_data_device_listener data_device_listener = {
    .data_offer = data_device_handle_data_offer,
    .enter      = data_device_handle_enter,
    .leave      = data_device_handle_leave,
    .motion     = data_device_handle_motion,
    .drop       = data_device_handle_drop,
    .selection  = data_device_handle_selection,
};

static void data_source_handle_target(void *data,
                                      struct wl_data_source *source,
                                      const char *mimetype)
{
    /* unused */
    (void)data;
    (void)mimetype;

    if (hadal.wl.selection_source != source) {
        amw_log_error("Wayland unknown clipboard data source");
        return;
    }
}

static void data_source_handle_send(void *data,
                                    struct wl_data_source *source,
                                    const char *mimetype,
                                    int fd)
{
    /* ignore if it's an outdated or invalid request */
    if (hadal.wl.selection_source != source || strcmp(mimetype, "text/plain;charset=utf-8") != 0) {
        close(fd);
        return;
    }

    /* unused */
    (void)data;

    char *str = hadal.wl.clipboard_string;
    size_t len = strlen(str);

    while (len > 0) {
        const ssize_t res = write(fd, str, len);
        if (res == -1) {
            if (errno == EINTR)
                continue;
            amw_log_error("Wayland error while writing clipboard string: %s", strerror(errno));
            break;
        }
        len -= res;
        str += res;
    }
    close(fd);
}

static void data_source_handle_cancelled(void *data, struct wl_data_source *source)
{
    /* unused */
    (void)data;

    wl_data_source_destroy(source);
    if (hadal.wl.selection_source != source)
        return;
    hadal.wl.selection_source = NULL;
}

static const struct wl_data_source_listener data_source_listener = {
    .target             = data_source_handle_target,
    .send               = data_source_handle_send,
    .cancelled          = data_source_handle_cancelled,
};

static void xdg_activation_handle_done(void *data,
                                       struct xdg_activation_token_v1 *activation_token,
                                       const char *token)
{
    amw_window_t *window = data;

    if (activation_token != window->wl.activation_token)
        return;

    xdg_activation_v1_activate(hadal.wl.activation_manager, token, window->wl.surface);
    xdg_activation_token_v1_destroy(window->wl.activation_token);
    window->wl.activation_token = NULL;
}

static const struct xdg_activation_token_v1_listener xdg_activation_listener = {
    .done = xdg_activation_handle_done,
};

void hadal_wayland_set_clipboard_string(const char *string)
{
    if (hadal.wl.selection_source) {
        wl_data_source_destroy(hadal.wl.selection_source);
        hadal.wl.selection_source = NULL;
    }

    char *copy = amw_arena_strdup(&hadal.clipboard_arena, string);
    if (!copy)
        return;
    
    hadal.wl.clipboard_string = copy;
    hadal.wl.selection_source = wl_data_device_manager_create_data_source(hadal.wl.data_device_manager);
    if (!hadal.wl.selection_source) {
        amw_log_error("Wayland failed to create a clipboard data source");
        amw_arena_reset(&hadal.clipboard_arena);
        return;
    }

    wl_data_source_add_listener(hadal.wl.selection_source, &data_source_listener, NULL);
    wl_data_source_offer(hadal.wl.selection_source, "text/plain;charset=utf-8");
    wl_data_device_set_selection(hadal.wl.data_device, hadal.wl.selection_source, hadal.wl.serial);
}

const char *hadal_wayland_get_clipboard_string(void)
{
    if (!hadal.wl.selection_offer) {
        amw_log_debug("No clipboard data available");
        return NULL;
    }
    if (hadal.wl.selection_source)
        return hadal.wl.clipboard_string;

    amw_arena_reset(&hadal.clipboard_arena);
    hadal.wl.clipboard_string = read_data_offer_as_string(hadal.wl.selection_offer, "text/plain;charset=utf-8");
    return hadal.wl.clipboard_string;
}

void hadal_wayland_update_buffer_scale_from_outputs(amw_window_t *window)
{

    if (!hadal.wl.compositor)
        amw_log_error(" 123 co do kurwy");
    if (wl_compositor_get_version(hadal.wl.compositor) < WL_SURFACE_SET_BUFFER_SCALE_SINCE_VERSION)
        return;
    if (!hadal.wl.compositor)
        amw_log_error(" 321 co do kurwy");
    if (!window->scale_framebuffer)
        return;
    if (!window->wl.fractional_scale)
        return;

    int32_t maxscale = 1;

    for (size_t i = 0; i < window->wl.output_scale_count; i++)
        maxscale = amw_max(window->wl.output_scales[i].factor, maxscale);

    /* only change the framebuffer size if the scale changed */
    if (window->wl.buffer_scale != maxscale) {
        window->wl.buffer_scale = maxscale;
        wl_surface_set_buffer_scale(window->wl.surface, maxscale);
        // TODO input window content scale
        resize_framebuffer(window);

        //if (window->visible)
            // TODO input window damage;
    }
}

bool hadal_wayland_create_window(amw_window_t *window, int32_t width, int32_t height)
{
    if (!create_native_surface(window, width, height))
        return AMW_FALSE;
    amw_log_debug("native surface created");

    if (window->output || window->visible) {
        amw_log_debug("creating xdg shell objects");
        if (!create_xdg_shell_objects(window))
            return AMW_FALSE;
    }
    amw_log_debug("wayland window created");
    return AMW_TRUE;
}

void hadal_wayland_destroy_window(amw_window_t *window)
{
    if (window == hadal.wl.pointer_focus)
        hadal.wl.pointer_focus = NULL;
    if (window == hadal.wl.keyboard_focus)
        hadal.wl.keyboard_focus = NULL;
    if (window->wl.fractional_scale)
        wp_fractional_scale_v1_destroy(window->wl.fractional_scale);
    if (window->wl.scaling_viewport)
        wp_viewport_destroy(window->wl.scaling_viewport);
    if (window->wl.activation_token)
        xdg_activation_token_v1_destroy(window->wl.activation_token);
    if (window->wl.idle_inhibitor)
        zwp_idle_inhibitor_v1_destroy(window->wl.idle_inhibitor);
    if (window->wl.relative_pointer)
        zwp_relative_pointer_v1_destroy(window->wl.relative_pointer);
    if (window->wl.locked_pointer)
        zwp_locked_pointer_v1_destroy(window->wl.locked_pointer);
    if (window->wl.confined_pointer)
        zwp_confined_pointer_v1_destroy(window->wl.confined_pointer);

    destroy_shell_objects(window);

    if (window->wl.surface)
        wl_surface_destroy(window->wl.surface);

    amw_free(window->wl.app_id);
    amw_free(window->wl.output_scales);
}

void hadal_wayland_show_window(amw_window_t *window)
{
    if (!window->wl.xdg.toplevel) {
        window->visible = AMW_TRUE;
        create_xdg_shell_objects(window);
    }
}

void hadal_wayland_hide_window(amw_window_t *window)
{
    if (window->visible) {
        window->visible = AMW_FALSE;
        destroy_shell_objects(window);
        wl_surface_attach(window->wl.surface, NULL, 0, 0);
        wl_surface_commit(window->wl.surface);
    }
}

void hadal_wayland_retitle_window(amw_window_t *window, const char *title)
{
    if (window->wl.xdg.toplevel)
        xdg_toplevel_set_title(window->wl.xdg.toplevel, title);
}

void hadal_wayland_resize_window(amw_window_t *window, int32_t width, int32_t height)
{
    if (window->output) {
        /* doensn't matter here i guess */
    } else {
        if (!resize_window(window, width, height))
            return;

        //if (window->visible)
        // TODO window damage callback
    } 
}

void hadal_wayland_window_size(amw_window_t *window, int32_t *width, int32_t *height)
{
    if (width) 
        *width = window->wl.width;
    if (height)
        *height = window->wl.height;
}

void hadal_wayland_framebuffer_size(amw_window_t *window, int32_t *width, int32_t *height)
{
    if (width)
        *width = window->wl.fb_width;
    if (height)
        *height = window->wl.fb_height;
}

void hadal_wayland_content_scale(amw_window_t *window, float *xscale, float *yscale)
{
    if (window->wl.fractional_scale) {
        if (xscale)
            *xscale = (float)window->wl.scaling_numerator / 120.f;
        if (yscale)
            *yscale = (float)window->wl.scaling_numerator / 120.f;
    } else {
        if (xscale)
            *xscale = (float)window->wl.buffer_scale;
        if (yscale)
            *yscale = (float)window->wl.buffer_scale;
    }
}

void hadal_wayland_add_seat_listener(struct wl_seat *seat)
{
    wl_seat_add_listener(seat, &seat_listener, NULL);
}

void hadal_wayland_add_data_device_listener(struct wl_data_device *device)
{
    wl_data_device_add_listener(device, &data_device_listener, NULL);
}
