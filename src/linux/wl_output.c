#include "../core/hadal.h"

#include <unistd.h>

static void output_handle_geometry(void *data,
                                   struct wl_output *wl_output,
                                   int32_t x,
                                   int32_t y,
                                   int32_t physical_width,
                                   int32_t physical_height,
                                   int32_t subpixel,
                                   const char *make,
                                   const char *model,
                                   int32_t transform)
{
    amw_output_t *output = data;

    /* unused */
    (void)wl_output;
    (void)subpixel;
    (void)transform;

    output->wl.x = x;
    output->wl.y = y;
    output->width_mm = physical_width;
    output->height_mm = physical_height;

    if (strlen(output->name) == 0)
        snprintf(output->name, sizeof(output->name) - 1, "%s %s", make, model);
}

static void output_handle_mode(void *data,
                               struct wl_output *wl_output,
                               uint32_t flags,
                               int32_t width,
                               int32_t height,
                               int32_t refresh)
{
    amw_output_t *output= data;
    amw_vidmode_t mode;

    /* unused */
    (void)wl_output;

    mode.width = width;
    mode.height = height;
    mode.red_bits = 8;
    mode.green_bits = 8;
    mode.blue_bits = 8;
    mode.refresh_rate = (int32_t)round(refresh / 1000.0);

    output->mode_count++;
    output->modes = amw_realloc(output->modes, output->mode_count * sizeof(amw_vidmode_t));
    output->modes[output->mode_count - 1] = mode;

    if (flags & WL_OUTPUT_MODE_CURRENT)
        output->wl.current_mode = output->mode_count - 1;
}

static void output_handle_done(void *data, struct wl_output *wl_output)
{
    amw_output_t *output = data;

    /* unused */
    (void)wl_output;

    if (output->width_mm <= 0 || output->height_mm <= 0) {
        /* if Wayland does not provide a physical size, assume the defauly 96 DPI */
        const amw_vidmode_t *mode = &output->modes[output->wl.current_mode];
        output->width_mm = (int32_t)(mode->width * 25.4f / 96.f);
        output->height_mm = (int32_t)(mode->height * 25.4f / 96.f);
    }

    for (int32_t i = 0; i < hadal.output_count; i++) {
        if (hadal.outputs[i] == output) return;
    }
    hadal_create_output(output, HADAL_CONNECTED, HADAL_INSERT_LAST);
}

static void output_handle_scale(void *data, struct wl_output *wl_output, int32_t factor)
{
    amw_output_t *output = data;

    /* unused */
    (void)wl_output;

    output->wl.scale = factor;

    for (amw_window_t *window = hadal.window_list; window; window = window->next) {
        for (size_t i = 0; i < window->wl.output_scale_count; i++) {
            if (window->wl.output_scales[i].output == output->wl.output) {
                window->wl.output_scales[i].factor = output->wl.scale;
                hadal_wayland_update_buffer_scale_from_outputs(window);
                break;
            }
        }
    }
}

static void output_handle_name(void *data, struct wl_output *wl_output, const char *name)
{
    amw_output_t *output = data;

    /* unused */
    (void)wl_output;

    strncpy(output->name, name, sizeof(output->name) - 1);
}

static void output_handle_description(void *data, struct wl_output *output, const char *description)
{
    (void)data;
    (void)output;
    (void)description;
}

static const struct wl_output_listener output_listener = {
    .geometry = output_handle_geometry,
    .mode = output_handle_mode,
    .done = output_handle_done,
    .scale = output_handle_scale,
    .name = output_handle_name,
    .description = output_handle_description,
};

void hadal_wayland_add_output(uint32_t name, uint32_t version)
{
    if (version < 2) {
        amw_log_error("Unsupported Wayland output interface version, name: %d, version: %d", name, version);
        return;
    }
    version = amw_min(version, WL_OUTPUT_NAME_SINCE_VERSION);

    struct wl_output *wl_output = wl_registry_bind(hadal.wl.registry, name, &wl_output_interface, version);
    if (!wl_output) {
        amw_log_debug("No Wayland output was added, name: %d, version: %d", name, version);
        return;
    }

    /* the actual output name will be set in the geometry handler */
    amw_output_t *output = hadal_alloc_output("", 0, 0);
    output->wl.scale = 1;
    output->wl.output = wl_output;
    output->wl.name = name;

    wl_proxy_set_tag((struct wl_proxy *)wl_output, &hadal.wl.tag);
    wl_output_add_listener(wl_output, &output_listener, output);
    amw_log_verbose("Added a Wayland output");
}

void hadal_wayland_free_output(amw_output_t *output)
{
    if (output->wl.output)
        wl_output_destroy(output->wl.output);
}

amw_vidmode_t *hadal_wayland_video_modes(amw_output_t *output, int32_t *count)
{
    *count = output->mode_count;
    return output->modes;
}

bool hadal_wayland_video_mode(amw_output_t *output, amw_vidmode_t *mode)
{
    *mode = output->modes[output->wl.current_mode];
    return AMW_TRUE;
}

void hadal_wayland_get_output_position(amw_output_t *output, int32_t *xpos, int32_t *ypos)
{
    if (xpos)
        *xpos = output->wl.x;
    if (ypos)
        *ypos = output->wl.y;
}
void hadal_wayland_get_output_content_scale(amw_output_t *output, float *xscale, float *yscale)
{
    if (xscale)
        *xscale = (float)output->wl.scale;
    if (yscale)
        *yscale = (float)output->wl.scale;
}

void hadal_wayland_get_output_workarea(amw_output_t *output, int32_t *xpos, int32_t *ypos, int32_t *width, int32_t *height)
{
    if (xpos)
        *xpos = output->wl.x;
    if (ypos)
        *ypos = output->wl.y;
    if (width)
        *width = output->modes[output->wl.current_mode].width;
    if (height)
        *height = output->modes[output->wl.current_mode].height;
}
