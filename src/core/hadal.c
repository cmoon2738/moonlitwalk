#include "moonlitwalk/amw.h"
#include "moonlitwalk/hadopelagic.h"
#include "hadal.h"

hadopelagic_t hadal = {0};

#define HADAL_STICK 3

static const char *platform_string(uint32_t id)
{
    switch (id) {
        case AMW_HADAL_PLATFORM_WIN32: return "win32";
        case AMW_HADAL_PLATFORM_COCOA: return "cocoa";
        case AMW_HADAL_PLATFORM_IOS: return "ios";
        case AMW_HADAL_PLATFORM_ANDROID: return "android";
        case AMW_HADAL_PLATFORM_WAYLAND: return "wayland";
        case AMW_HADAL_PLATFORM_XCB: return "xcb";
        case AMW_HADAL_PLATFORM_KMS: return "kms";
        case AMW_HADAL_PLATFORM_HEADLESS: return "headless";
        default: break;
    }
    return NULL;
}

static const struct { int32_t id; bool (*connect)(void); } supported_platforms[] = {
#if defined(MW_PLATFORM_WINDOWS)
    /* TODO */
#elif defined(MW_PLATFORM_MACOSX)
    /* TODO */
#elif defined(MW_PLATFORM_IOS)
    /* TODO */
#elif defined(MW_PLATFORM_ANDROID)
    /* TODO */
#elif defined(MW_PLATFORM_EMSCRIPTEN)
    /* TODO */
#endif
#ifdef AMW_NATIVE_WAYLAND
    { AMW_HADAL_PLATFORM_WAYLAND, hadal_wayland_connect },
#endif
#ifdef AMW_NATIVE_XCB
    /* TODO */
#endif
#ifdef AMW_NATIVE_KMS
    /* TODO */
#endif
};

static bool select_platform(int32_t id) 
{
    const size_t count = amw_arraysize(supported_platforms);

    if (id != AMW_HADAL_ANY_PLATFORM &&
        id != AMW_HADAL_PLATFORM_WIN32 &&
        id != AMW_HADAL_PLATFORM_COCOA &&
        id != AMW_HADAL_PLATFORM_IOS &&
        id != AMW_HADAL_PLATFORM_ANDROID &&
        id != AMW_HADAL_PLATFORM_WAYLAND &&
        id != AMW_HADAL_PLATFORM_XCB &&
        id != AMW_HADAL_PLATFORM_KMS &&
        id != AMW_HADAL_PLATFORM_HEADLESS)
    {
        amw_log_error("invalid platform ID '%X : %s'", id, platform_string(id));
        return AMW_FALSE;
    }

    /* only allow headless mode if explicitly requested */
    if (id == AMW_HADAL_PLATFORM_HEADLESS) {
        /* TODO headless connect */
        // return AMW_TRUE
        amw_log_error("Headless mode not supported right now.");
        return AMW_FALSE;
    } else if (count == 0) {
        amw_log_error("This binary supports only headless platform. Headless mode must be called explicitly.");
        return AMW_FALSE;
    }

    if (id == AMW_HADAL_ANY_PLATFORM) {
        if (count == 1)
            return supported_platforms[0].connect();
        
        for (size_t i = 0; i < count; i++) {
            if (supported_platforms[i].connect())
                return AMW_TRUE;
        }
        amw_log_error("Failed to detect any supported platform.");
    } else {
        for (size_t i = 0; i < count; i++) {
            if (supported_platforms[i].id == id)
                return supported_platforms[i].connect();
        }
        amw_log_error("The requested platform is not supported");
    }
    return AMW_FALSE;
}

static void terminate(void)
{
    if (!hadal.initialized)
        return;

    amw_log_verbose("Terminating Hadal...");
    if (hadal.window_list != NULL)
        amw_hadal_destroy_window(hadal.window_list);

    if (hadal.api.terminate)
        hadal.api.terminate();

    if (hadal.mutex) {
        amw_mutex_destroy(hadal.mutex);
    }

    amw_arena_free(&hadal.temp_arena);
    amw_arena_free(&hadal.clipboard_arena);
    amw_zero(hadal);
    amw_log_verbose("Hadal terminated!");
}

int32_t amw_hadal_init(int32_t platform_id)
{
    if (hadal.initialized)
        return AMW_SUCCESS;

    amw_log_verbose("Initializing Hadal...");
    amw_zero(hadal);

    if (platform_id <= 0)
        platform_id = AMW_HADAL_ANY_PLATFORM;

    if (!select_platform(platform_id))
        return -1; /* TODO error result code */

    hadal.mutex = amw_mutex_create();
    if (!hadal.mutex) {
        amw_log_error("Could not create a mutex for hadal!");
        terminate();
        return -1; /* TODO error result code */
    }
    hadal.api.init();

    hadal.initialized = AMW_TRUE;
    amw_log_verbose("Hadal initialized!");

    return AMW_SUCCESS;
}

void amw_hadal_terminate(void)
{
    if (hadal.initialized) {
        terminate();
    }
}

amw_window_t *amw_hadal_create_window(const char *title, int32_t width, int32_t height, uint32_t flags)
{
    if (!hadal.initialized)
        return NULL;

    /* FIXME, for now allow only one window */
    if (hadal.window_list != NULL)
        return hadal.window_list;

    amw_assert(title != NULL);

    if (width <= 0 || height <= 0) {
        amw_log_error("Invalid window dimensions %ix%i", width, height);
        return NULL;
    }
    amw_log_verbose("Creating a window: %ix%i '%s'", width, height, title);

    amw_window_t *window = (amw_window_t *)amw_malloc(sizeof(amw_window_t));
    if (!window) {
        amw_log_error("Failed to allocate resources for a new window");
        return NULL;
    }
    amw_zerop(window);

    window->title     = strdup(title);
    window->output    = NULL;
    window->minwidth  = HADAL_DONT_CARE;
    window->minheight = HADAL_DONT_CARE;
    window->maxwidth  = HADAL_DONT_CARE;
    window->maxheight = HADAL_DONT_CARE;
    window->numer     = HADAL_DONT_CARE;
    window->denom     = HADAL_DONT_CARE;

    window->maximized         = flags & AMW_WINDOW_FLAG_MAXIMIZED;
    window->resizable         = flags & AMW_WINDOW_FLAG_RESIZABLE;
    window->transparent       = flags & AMW_WINDOW_FLAG_TRANSPARENT;
    window->auto_iconify      = flags & AMW_WINDOW_FLAG_AUTO_ICONIFY;
    window->scale_framebuffer = flags & AMW_WINDOW_FLAG_SCALE_FRAMEBUFFER;
    /* FIXME at any time this probably should be updated huh */

    if (!hadal.api.create_window(window, width, height)) {
        amw_hadal_destroy_window(window);
        return NULL;
    }

    hadal.window_list = window;
    return window;
}

void amw_hadal_destroy_window(amw_window_t *window)
{
    if (window != NULL && hadal.initialized) {
        hadal.api.destroy_window(window);
        hadal.window_list = NULL; /* FIXME only one window allowed now */
        amw_free(window->title);
        amw_free(window);
    }
}

bool amw_hadal_should_close(amw_window_t *window)
{
    amw_assert(window);
    return window->should_close;
}

void amw_hadal_request_close(amw_window_t *window, bool request)
{
    amw_assert(window);
    window->should_close = request;
}

uint32_t amw_hadal_platform(void)
{
    if (hadal.initialized)
        return hadal.api.id;
    return 0;
}

bool amw_hadal_platform_supported(int32_t id)
{
    const size_t count = amw_arraysize(supported_platforms);
    size_t i;

    if (id == AMW_HADAL_PLATFORM_HEADLESS)
        return AMW_TRUE;
    for (i = 0; i < count; i++) {
        if (id == supported_platforms[i].id)
            return AMW_TRUE;
    }
    return AMW_FALSE;
}

void amw_hadal_show_window(amw_window_t *window)
{
    if (!hadal.initialized)
        return;

    amw_assert(window);

    if (window->output)
        return;

    hadal.api.show_window(window);
    if (window->focus_on_show) {
        // TODO focus window
    }
}

void amw_hadal_hide_window(amw_window_t *window)
{
    if (!hadal.initialized)
        return;

    amw_assert(window);

    if (window->output)
        return;
    
    hadal.api.hide_window(window);
}

void amw_hadal_retitle_window(amw_window_t *window, const char *title)
{
    if (!hadal.initialized)
        return;

    amw_assert(window);
    amw_assert(title);

    char *prev = window->title;
    window->title = strdup(title);
    hadal.api.retitle_window(window, title);
    amw_free(prev);
}

void amw_hadal_resize_window(amw_window_t *window, int32_t width, int32_t height)
{
    if (!hadal.initialized)
        return;

    amw_assert(window);
    amw_assert(width >= 0);
    amw_assert(height >= 0);

    window->vidmode.width  = width;
    window->vidmode.height = height;
    hadal.api.resize_window(window, width, height);
}

void amw_hadal_window_size(amw_window_t *window, int32_t *width, int32_t *height)
{
    if (width)
        *width = 0;
    if (height)
        *height = 0;

    if (!hadal.initialized)
        return;
    amw_assert(window);

    hadal.api.window_size(window, width, height);
}

void amw_hadal_framebuffer_size(amw_window_t *window, int32_t *width, int32_t *height)
{
    if (width)
        *width = 0;
    if (height)
        *height = 0;

    if (!hadal.initialized)
        return;
    amw_assert(window);

    hadal.api.framebuffer_size(window, width, height);
}

void amw_hadal_content_scale(amw_window_t *window, float *xscale, float *yscale)
{
    if (xscale)
        *xscale = 0;
    if (yscale)
        *yscale = 0;

    if (!hadal.initialized)
        return;
    amw_assert(window);

    hadal.api.content_scale(window, xscale, yscale);
}

void amw_hadal_set_clipboard_string(const char *string)
{
    amw_assert(string != NULL);

    if (!hadal.initialized) return;
    hadal.api.set_clipboard_string(string);
}

const char *amw_hadal_get_clipboard_string(void)
{
    if (!hadal.initialized) return NULL;
    return hadal.api.get_clipboard_string();
}

/* renderer/vulkan.h */
#ifdef AMW_NATIVE_VULKAN
bool amw_vk_physical_device_presentation_support(VkPhysicalDevice device, uint32_t queue_family)
{
    if (!hadal.initialized)
        return AMW_FALSE;

    amw_assert(device != VK_NULL_HANDLE);

    return hadal.api.physical_device_presentation_support(device, queue_family);
}

VkResult amw_vk_surface_create(VkInstance instance, 
                               amw_window_t *window, 
                               const VkAllocationCallbacks *allocator, 
                               VkSurfaceKHR *surface)
{
    amw_assert(surface != NULL);
    *surface = VK_NULL_HANDLE;

    if (!hadal.initialized)
        return VK_ERROR_INITIALIZATION_FAILED;

    amw_assert(window != NULL);
    amw_assert(instance != VK_NULL_HANDLE);

    return hadal.api.create_surface(instance, window, allocator, surface);
}
#endif /* AMW_NATIVE_VULKAN */

/*
 * INTERNAL
 */

char **hadal_parse_uri_list(char *text, int32_t *count)
{
    const char *prefix = "file://";
    char      **paths = NULL;
    char       *line;

    *count = 0;

    while ((line = strtok(text, "\r\n"))) {
        char *path;
        text = NULL;

        if (line[0] == '#')
            continue;

        if (strncmp(line, prefix, strlen(prefix)) == 0) {
            line += strlen(prefix);
            while (*line != '/')
                line++;
        }
        (*count)++;

        path = amw_calloc(strlen(line) + 1, 1);
        paths = amw_realloc(paths, *count * sizeof(char *));
        paths[*count - 1] = path;

        while (*line) {
            if (line[0] == '%' && line[1] && line[2]) {
                const char digits[3] = { line[1], line[2], '\0' };
                *path = (char)strtol(digits, NULL, 16);
                line += 2;
            } else {
                *path = *line;
            }
            path++;
            line++;
        }
    }
    return paths;
}


/* when i make updates to the api, 
 * it basically checks if i implemented everything 
 * for the target platform i'm testing in */
bool hadal_debug_check_api_uptodate(const hadal_api_t *api)
{
    bool out = AMW_TRUE;

#ifdef AMW_DEBUG
    amw_assert(api != NULL);
    const char *plat = platform_string(api->id);

#define HAPICHECK(fn) if (api->fn == NULL) { amw_log_warn("Missing call in Hadal internal api - '%s_%s'", plat, #fn); out = AMW_FALSE; }
    HAPICHECK(init)
    HAPICHECK(terminate)
    HAPICHECK(free_output)
    HAPICHECK(video_modes)
    HAPICHECK(video_mode)
    HAPICHECK(get_output_position)
    HAPICHECK(get_output_content_scale)
    HAPICHECK(get_output_workarea)
    HAPICHECK(set_clipboard_string)
    HAPICHECK(get_clipboard_string)
    HAPICHECK(create_window)
    HAPICHECK(destroy_window)
    HAPICHECK(show_window)
    HAPICHECK(hide_window)
    HAPICHECK(retitle_window)
    HAPICHECK(resize_window)
    HAPICHECK(window_size)
    HAPICHECK(framebuffer_size)
    HAPICHECK(content_scale)
#ifdef AMW_NATIVE_VULKAN
    HAPICHECK(physical_device_presentation_support)
    HAPICHECK(create_surface)
#endif /* AMW_NATIVE_VULKAN */
#undef HAPICHECK
#endif /* AMW_DEBUG */
    return out;
}

/* lexically compare video modes */
static int32_t compare_vidmodes(const void *fp, const void *sp)
{
    const amw_vidmode_t *fm = fp;
    const amw_vidmode_t *sm = sp;
    const int32_t fbpp = fm->red_bits + fm->green_bits + fm->blue_bits;
    const int32_t sbpp = sm->red_bits + sm->green_bits + sm->blue_bits;
    const int32_t farea = fm->width * fm->height;
    const int32_t sarea = sm->width * sm->height;

    /* sort on color bits per pixel */
    if (fbpp != sbpp) return fbpp - sbpp;

    /* sort on screen area */
    if (farea != sarea) return farea - sarea;

    /* sort on width */
    if (fm->width != sm->width) return fm->width - sm->width;

    /* sort on refresh rate */
    return fm->refresh_rate - sm->refresh_rate;
}

/* available modes for the specified output */
static bool refresh_vidmodes(amw_output_t *output)
{
    int32_t mode_count;
    amw_vidmode_t *modes;

    if (output->modes) 
        return AMW_TRUE;

    modes = hadal.api.video_modes(output, &mode_count);
    if (!modes)
        return AMW_FALSE;

    qsort(modes, mode_count, sizeof(amw_vidmode_t), compare_vidmodes);

    amw_free(output->modes);
    output->modes = modes;
    output->mode_count = mode_count;

    return AMW_TRUE;
}

const amw_vidmode_t *hadal_choose_video_mode(amw_output_t *output, const amw_vidmode_t *desired)
{
    int32_t i;
    uint32_t size_diff, least_size_diff = UINT32_MAX;
    uint32_t rate_diff, least_rate_diff = UINT32_MAX;
    uint32_t color_diff, least_color_diff = UINT32_MAX;
    const amw_vidmode_t *current;
    const amw_vidmode_t *closest = NULL;

    if (!refresh_vidmodes(output))
        return NULL;

    for (i = 0; i < output->mode_count; i++) {
        current = output->modes + i;
        color_diff = 0;

        if (desired->red_bits != -1)
            color_diff += abs(current->red_bits - desired->red_bits);
        if (desired->green_bits != -1)
            color_diff += abs(current->green_bits - desired->green_bits);
        if (desired->blue_bits != -1)
            color_diff += abs(current->blue_bits - desired->blue_bits);

        size_diff = abs((current->width - desired->width) * (current->width - desired->width) +
                        (current->height - desired->height) * (current->height - desired->height));

        if (desired->refresh_rate != -1) {
            rate_diff = abs(current->refresh_rate - desired->refresh_rate);
        } else {
            rate_diff = UINT32_MAX - current->refresh_rate;
        }

        if ((color_diff < least_color_diff) || 
            (color_diff == least_color_diff && size_diff < least_size_diff) ||
            (color_diff == least_color_diff && size_diff == least_size_diff && rate_diff < least_rate_diff))
        {
            closest = current;
            least_size_diff = size_diff;
            least_rate_diff = rate_diff;
            least_color_diff = color_diff;
        }
    }
    return closest;
}

void hadal_input_output(amw_output_t *output, int32_t action, int32_t placement)
{
    amw_assert(output != NULL);
    amw_assert(action == HADAL_CONNECTED || action == HADAL_DISCONNECTED);
    amw_assert(placement == HADAL_INSERT_FIRST || placement == HADAL_INSERT_LAST);

    /* FIXME only one window allowed for now */
    amw_window_t *window = hadal.window_list;

    if (action == HADAL_CONNECTED) {
        hadal.output_count++;
        hadal.outputs = amw_realloc(hadal.outputs, sizeof(amw_output_t *) * hadal.output_count);

        if (placement == HADAL_INSERT_FIRST) {
            amw_memmove(hadal.outputs + 1, hadal.outputs, ((size_t)hadal.output_count - 1) * sizeof(amw_output_t *));
            hadal.outputs[0] = output;
        } else {
            hadal.outputs[hadal.output_count - 1] = output;
        }
    } else if (action == HADAL_DISCONNECTED) {
        if (window->output == output) {
            //TODO handle the *output in window struct
        }

        for (int32_t i = 0; i < hadal.output_count; i++) {
            if (hadal.outputs[i] == output) {
                hadal.output_count--;
                amw_memmove(hadal.outputs + i, hadal.outputs + i + 1, 
                    ((size_t)hadal.output_count - i) * sizeof(amw_output_t *));
                break;
            }
        }
    }
    if (action == HADAL_DISCONNECTED)
        hadal_free_output(output);
}

amw_output_t *hadal_alloc_output(const char *name, int32_t width_mm, int32_t height_mm)
{
    amw_output_t *output = amw_malloc(sizeof(amw_output_t));
    output->width_mm = width_mm;
    output->height_mm = height_mm;

    strncpy(output->name, name, sizeof(output->name) - 1);
    return output;
}

void hadal_free_output(amw_output_t *output)
{
    if (output == NULL) return;
    hadal.api.free_output(output);
}

/*
 * INPUT CALLBACKS
 */

void hadal_input_key_callback(amw_window_t *window, int32_t key, int32_t scancode, int32_t action, int32_t mods)
{
    amw_assert(window != NULL);
    amw_assert(key >= 0 || key == AMW_KEY_INVALID);
    amw_assert(key <= AMW_KEY_LAST);
    amw_assert(action == AMW_INPUT_PRESS || action == AMW_INPUT_RELEASE);
    amw_assert(mods > AMW_KEYMOD_INVALID && mods <= AMW_KEYMOD_NUM_LOCK);

    if (key >= 0 && key <= AMW_KEY_LAST) {
        bool repeated = AMW_FALSE;

        if (action == AMW_INPUT_RELEASE && window->keys[key] == AMW_INPUT_RELEASE)
            return;
        if (action == AMW_INPUT_PRESS && window->keys[key] == AMW_INPUT_PRESS)
            repeated = AMW_TRUE;
        if (action == AMW_INPUT_RELEASE && window->sticky_keys)
            window->keys[key] = HADAL_STICK;
        else
            window->keys[key] = (char)action;
        if (repeated)
            action = AMW_INPUT_REPEAT;
    }
    if (!window->lock_key_mods)
        mods &= ~(AMW_KEYMOD_CAPS_LOCK | AMW_KEYMOD_NUM_LOCK);
    if (window->input_callbacks.key)
        window->input_callbacks.key(window->input_callbacks.data, window, key, scancode, action, mods);
}

void amw_hadal_setup_input_callbacks(amw_window_t *window, amw_input_callbacks_t callbacks)
{
    amw_assert(window);
    window->input_callbacks = callbacks;
}

/*
 * RENDERER CALLBACKS
 */

void hadal_input_framebuffer_resized_callback(amw_window_t *window, int32_t width, int32_t height)
{
    amw_assert(window);
    amw_assert(width >= 0);
    amw_assert(height >= 0);

    if (window->renderer_callbacks.framebuffer_resized) {
        window->renderer_callbacks.framebuffer_resized(
            window->renderer_callbacks.data, 
            window, width, height);
    }
}

void amw_hadal_setup_renderer_callbacks(amw_window_t *window, amw_renderer_callbacks_t callbacks)
{
    amw_assert(window);
    window->renderer_callbacks = callbacks;
}

