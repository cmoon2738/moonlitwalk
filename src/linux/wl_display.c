#include "../core/hadal.h"
#include "wayland.h"

#include <sys/timerfd.h>

static void wm_base_handle_ping(void *data,
                                struct xdg_wm_base *shell,
                                uint32_t serial)
{
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
    (void)data;

    if (strcmp(interface, "wl_shm") == 0) {
        hadal.wl.shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    } else if (strcmp(interface, "wl_seat") == 0) {
        if (!hadal.wl.seat) {
            hadal.wl.seat = wl_registry_bind(registry, name, &wl_seat_interface, amw_min(4, version));
            // TODO seat listener
            amw_log_debug("implement wayland seat listener");
        }
    } else if (strcmp(interface, "xdg_wm_base") == 0) {
        hadal.wl.shell = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(hadal.wl.shell, &xdg_wm_base_listener, NULL);
    }
}

static void handle_registry_global_remove(void *data, 
                                          struct wl_registry *registry, 
                                          uint32_t name)
{
    (void)data;
    (void)registry;
    (void)name;
}

static const struct wl_registry_listener registry_listener = {
    .global        = handle_registry_global,
    .global_remove = handle_registry_global_remove,
};


bool hadal_wayland_connect(void)
{
    amw_log_verbose("Trying to connect to a Wayland display...");

    const hadal_api_t wayland = {
        .id = AMW_HADAL_PLATFORM_WAYLAND,
        .init = hadal_wayland_init,
        .terminate = hadal_wayland_terminate,
        .create_window = hadal_wayland_create_window,
        .destroy_window = hadal_wayland_destroy_window,
        .physical_device_presentation_support = hadal_wayland_physical_device_presentation_support,
        .create_surface = hadal_wayland_create_surface,
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
    hadal.wl.registry = wl_display_get_registry(hadal.wl.display);
    wl_registry_add_listener(hadal.wl.registry, &registry_listener, NULL);

    // TODO keytables
    //hadal.wl.xkb.context = xkb_context_new(0);
    
    /* sync to get all registry objects */
    wl_display_roundtrip(hadal.wl.display);

    /* FIXME implement wl_output in state (monitors)
     * sync to get all initial output events */
    //wl_display_roundtrip(hadal.wl.display);

    if (!hadal.wl.shell) {
        amw_log_error("Wayland compositor is missing xdg-wm-base protocol support.");
        return -1; /* TODO error result code */
    }

    if (!hadal.wl.shm) {
        amw_log_error("Wayland compositor is missing wl-shm.");
        return -1; /* TODO error result code */
    }

    return 0;
}

void hadal_wayland_terminate(void)
{
    if (hadal.wl.shm)
        wl_shm_destroy(hadal.wl.shm);
    if (hadal.wl.shell)
        xdg_wm_base_destroy(hadal.wl.shell);
    if (hadal.wl.seat)
        wl_seat_destroy(hadal.wl.seat);
    if (hadal.wl.registry)
        wl_registry_destroy(hadal.wl.registry);
    if (hadal.wl.display) {
        wl_display_flush(hadal.wl.display);
        wl_display_disconnect(hadal.wl.display);
    }
}

