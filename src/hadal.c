#include "amw.h"
#include "hadopelagic.h"
#include "internal.h"
#include "system.h"

hadopelagic_t hadal = {0};

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
    { AMW_HADAL_PLATFORM_WAYLAND, amw_wayland_connect },
#endif
#ifdef AMW_NATIVE_XCB
    /* TODO */
#endif
#ifdef AMW_NATIVE_KMS
    /* TODO */
#endif
};
amw_static_assert(amw_arraysize(supported_platforms) > 0, "There should be atleast ONE supported platform here...");

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
        return AMW_TRUE;
    } else if (count == 0) {
        amw_log_fatal("This binary supports only headless platform. Headless mode must be called explicitly.");
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
    amw_assert_paranoid(!"unreachable code");
    return AMW_FALSE;
}

static void terminate(void)
{
    if (hadal.window_list)
        amw_window_destroy(hadal.window_list);

    if (hadal.api.terminate)
        hadal.api.terminate();

    if (hadal.mutex)
        amw_mutex_destroy(hadal.mutex);
    amw_zero(hadal);
}

int32_t amw_hadal_init(int32_t platform_id)
{
    if (hadal.initialized)
        return 0;

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

    hadal.initialized = AMW_TRUE;
    amw_log_verbose("Hadal initialized!");

    return 0;
}

void amw_hadal_terminate(void)
{
    if (hadal.initialized) {
        amw_log_verbose("Terminating Hadal...");
        terminate();
    }
}

amw_window_t *amw_window_create(const char *title, int32_t width, int32_t height, uint32_t flags)
{
    if (!hadal.initialized)
        return NULL;

    /* FIXME, for now allow only one window */
    if (hadal.window_list)
        return hadal.window_list;

    amw_assert(title != NULL);

    if (width <= 0 || height <= 0) {
        amw_log_error("Invalid window dimensions %ix%i", width, height);
        return AMW_FALSE;
    }

    amw_window_t *window = (amw_window_t *)amw_malloc(sizeof(amw_window_t));

    window->flags = flags;
    window->width = width;
    window->height = height;
    window->title = strdup(title);

    if (!hadal.api.window_create(window)) {
        amw_window_destroy(window);
        return NULL;
    }

    hadal.window_list = window;
    return window;
}

void amw_window_destroy(amw_window_t *window)
{
    if (window) {
        hadal.window_list = NULL; /* FIXME only one window allowed now */

        hadal.api.window_destroy(window);
        amw_free(window->title);
        amw_free(window);
    }
}

bool amw_window_should_close(amw_window_t *window)
{
    return window->should_close;
}

void amw_window_request_close(amw_window_t *window, bool request)
{
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

    return hadal.api.surface_create(instance, window, allocator, surface);
}

/* when i make updates to the api, 
 * it basically checks if i implemented everything 
 * for the target platform i'm testing in */
bool hadal_debug_check_api_uptodate(const hadal_api_t *api)
{
    amw_assert(api != NULL);

    bool out = AMW_TRUE;

#ifdef AMW_DEBUG
    const char *plat = platform_string(api->id);

#define HAPICHECK(fn) if (api->fn == NULL) { amw_log_warn("Missing call in Hadal internal api - '%s_%s'", plat, #fn); out = AMW_FALSE; }
    HAPICHECK(init)
    HAPICHECK(terminate)
    HAPICHECK(window_create)
    HAPICHECK(window_destroy)
    HAPICHECK(physical_device_presentation_support)
    HAPICHECK(surface_create)
#undef HAPICHECK
#endif /* AMW_DEBUG */
    return out;
}
