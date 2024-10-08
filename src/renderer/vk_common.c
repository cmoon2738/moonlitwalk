#include <moonlitwalk/amw.h>
#include <moonlitwalk/hadopelagic.h>

#include "vk.h"

#if AMW_BUILD_VALIDATION_LAYERS
static bool validation_layers_enabled = AMW_TRUE;
static const char *validation_layers[] = {
    "VK_LAYER_KHRONOS_validation"
};
#endif /* AMW_BUILD_VALIDATION_LAYERS */

const char *device_extensions[] = {
    "VK_KHR_swapchain",
    "VK_KHR_dynamic_rendering",
#if AMW_BUILD_VALIDATION_LAYERS
    "VK_KHR_shader_non_semantic_info",
#endif /* AMW_BUILD_VALIDATION_LAYERS */
};

static char *surface_extension_name(void)
{
    uint32_t platform_id = amw_hadal_platform();
    switch (platform_id) {
        case AMW_HADAL_ANY_PLATFORM: return NULL;
#if defined(AMW_PLATFORM_WINDOWS)
        case AMW_HADAL_PLATFORM_WIN32: return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#elif defined(AMW_PLATFORM_MACOSX)
        case AMW_HADAL_PLATFORM_COCOA: return VK_EXT_METAL_SURFACE_EXTENSION_NAME;
#elif defined(AMW_PLATFORM_IOS)
        case AMW_HADAL_PLATFORM_IOS: return VK_EXT_METAL_SURFACE_EXTENSION_NAME;
#elif defined(AMW_PLATFORM_ANDROID)
        case AMW_HADAL_PLATFORM_ANDROID: return VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
#endif
#ifdef AMW_NATIVE_WAYLAND
        case AMW_HADAL_PLATFORM_WAYLAND: return VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME; 
#endif
#ifdef MW_NATIVE_XCB
        case AMW_HADAL_PLATFORM_XCB: return VK_KHR_XCB_SURFACE_EXTENSION_NAME;
#endif
#ifdef MW_NATIVE_KMS
        case AMW_HADAL_PLATFORM_KMS: return VK_KHR_DISPLAY_EXTENSION_NAME;
#endif
        case AMW_HADAL_PLATFORM_HEADLESS: return VK_EXT_HEADLESS_SURFACE_EXTENSION_NAME;
        default: return NULL;
    }
}

static void instance_extensions(amw_arena_t *a, char **extensions, uint32_t *ext_count)
{
    /* FIXME its an abomination */

    /* check if optional instance extensions can be used */
    uint32_t instance_ext_count;
    AMW_VK_VERIFY(vkEnumerateInstanceExtensionProperties(NULL, &instance_ext_count, NULL));

    if (instance_ext_count > 0) {
        VkExtensionProperties *available_exts = (VkExtensionProperties *)amw_arena_alloc(a, sizeof(VkExtensionProperties) * instance_ext_count);
        AMW_VK_VERIFY(vkEnumerateInstanceExtensionProperties(NULL, &instance_ext_count, available_exts));

        for (uint32_t i = 0; i < instance_ext_count; i++) { 
                //strcmp(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, available_exts[i].extensionName) == 0;
#if AMW_BUILD_VALIDATION_LAYERS
                //strcmp(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, available_exts[i].extensionName) == 0;
#endif /* AMW_BUILD_VALIDATION_LAYERS */
        }
        //amw_free(available_exts);
    }

    if (extensions) {
        extensions[0] = VK_KHR_SURFACE_EXTENSION_NAME;
        extensions[1] = surface_extension_name();
        if (extensions[1] == NULL) {
            amw_log_error("Got a NULL surface extension name, fix your shit");
            return;
        }
        extensions[2] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
    }
    if (ext_count)
        *ext_count = 3;
}

static amw_result_t create_instance(amw_vulkan_t *vk)
{
    uint32_t instance_version = amw_vk_version();

#if AMW_BUILD_VALIDATION_LAYERS
    uint32_t layer_count = 0;
    vkEnumerateInstanceLayerProperties(&layer_count, NULL);

    if (layer_count == 0) {
        amw_log_debug("\tValidation layers are not supported in this device, they will be disabled");
        validation_layers_enabled = AMW_FALSE;
    }
    
    VkValidationFeatureEnableEXT validation_feature_enable[] = {
        VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT
    };
    VkValidationFeaturesEXT validation_features = {
        .sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
        .pNext = NULL,
        .enabledValidationFeatureCount = amw_arraysize(validation_feature_enable),
        .pEnabledValidationFeatures = validation_feature_enable,
        .disabledValidationFeatureCount = 0,
        .pDisabledValidationFeatures = NULL,
    };
#endif /* AMW_BUILD_VALIDATION_LAYERS */

    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = NULL,
        .pApplicationName = "Lake in the Lungs",
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = "A MoonlitWalk Engine",
        .engineVersion = amw_version,
        .apiVersion = instance_version,
    };

    uint32_t extension_count = 0;
    instance_extensions(&vk->temporary_arena, NULL, &extension_count);
    if (extension_count == 0) {
        amw_log_error("dupa");
        return -1; // TODO
    }

    char **wanted_extensions = (char **)amw_arena_alloc(&vk->temporary_arena, sizeof(const char *) * extension_count);
    instance_extensions(&vk->temporary_arena, wanted_extensions, NULL);

    amw_log_verbose("Enabled Vulkan extensions:");
    for (uint32_t i = 0; i < extension_count; i++) {
        amw_log_verbose(" %3d %s", i, wanted_extensions[i]);
    }

    VkInstanceCreateInfo create_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = NULL,
        .flags = 0,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .enabledExtensionCount = extension_count,
        .ppEnabledExtensionNames = (const char * const *)wanted_extensions,
    };

#if AMW_BUILD_VALIDATION_LAYERS
    if (validation_layers_enabled) {
        create_info.pNext = &validation_features;
        create_info.enabledLayerCount = 1;
        create_info.ppEnabledLayerNames = (const char * const *)validation_layers;
    }
#endif /* AMW_BUILD_VALIDATION_LAYERS */

    AMW_VK_VERIFY(vkCreateInstance(&create_info, NULL, &vk->instance));
    amw_vk_load_instance_pointers(vk->instance);

#if AMW_BUILD_VALIDATION_LAYERS
    if (validation_layers_enabled)
        amw_vk_create_validation_layers(vk->instance);
#endif /* AMW_BUILD_VALIDATION_LAYERS */

    amw_log_verbose("Created a Vulkan instance...");
    amw_arena_reset(&vk->temporary_arena);
    return AMW_SUCCESS;
}

/*
static bool_t device_extensions_supported(const VkPhysicalDevice *pd)
{
    uint32_t available_ext_count = 0;
    bool_t   vk_khr_swapchain_ext_available = MW_FALSE;
    MVK_VERIFY(vkEnumerateDeviceExtensionProperties(*pd, NULL, &available_ext_count, NULL));

    if (available_ext_count > 0) {
        VkExtensionProperties *extensions = (VkExtensionProperties *)mw_malloc(available_ext_count * sizeof(VkExtensionProperties));
        MVK_VERIFY(vkEnumerateDeviceExtensionProperties(*pd, NULL, &available_ext_count, extensions));

        for (uint32_t i = 0; i < available_ext_count; i++) {
            vk_khr_swapchain_ext_available |= strcmp(extensions[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0;
#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
			vk_config.vk_khr_portability_subset_available |= strcmp(extensions[i].extensionName, VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME) == 0;
#endif
        }
        mw_free(extensions);
    }

    / * lack of swapchain extension disqualifies the device * /
    return vk_khr_swapchain_ext_available;
}

static const char *device_type_string(VkPhysicalDeviceType pd_type)
{
#define DEVTYPESTR(r) case VK_ ##r: return "VK_"#r
    switch (pd_type) {
        DEVTYPESTR(PHYSICAL_DEVICE_TYPE_OTHER);
        DEVTYPESTR(PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU);
        DEVTYPESTR(PHYSICAL_DEVICE_TYPE_DISCRETE_GPU);
        DEVTYPESTR(PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU);
        DEVTYPESTR(PHYSICAL_DEVICE_TYPE_CPU);
        default: return "<unknown>";
    }
#undef DEVTYPESTR
    return "UNKNOWN DEVICE";
}

static const char *vendor_name_string(uint32_t vendor_id)
{
    switch (vendor_id) {
        case 0x1002: return "AMD";
        case 0x1010: return "ImgTec";
        case 0x106B: return "Apple";
        case 0x10DE: return "NVIDIA";
        case 0x13B5: return "ARM";
        case 0x5143: return "Qualcomm";
        case 0x8086: return "Intel";
        default: return "<unknown>";
    }
}

static void get_best_physical_device(const VkPhysicalDevice *devices, int32_t preferred_idx, int32_t count)
{
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures   device_features;
    uint32_t                   queue_family_count = 0;

    for (int32_t i = 0; i < count; i++) {
        vkGetPhysicalDeviceProperties(devices[i], &device_properties);
        vkGetPhysicalDeviceFeatures(devices[i], &device_features);
        vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queue_family_count, NULL);

        if (queue_family_count == 0)
            continue;

        / * prefer discrete GPU but if it's the only one available then don't be picky... 
        / * also, if the user specifies a preferred device, select it * /
        bool_t best_properties = device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        if (preferred_idx == i || (best_properties && preferred_idx < 0) || count == 1) {

            bool_t ext_supported = device_extensions_supported(&devices[i]);

            / * if no extensions, try next device * /
            if (!ext_supported)
                continue;

            VkQueueFamilyProperties *queue_families = (VkQueueFamilyProperties *)mw_malloc(queue_family_count * sizeof(VkQueueFamilyProperties));
            vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queue_family_count, queue_families);

            / * next check - device is OK if there is at least one queue with VK_QUEUE_GRAPHICS_BIT set * /
            for (uint32_t j = 0; j < queue_family_count; j++) {
                / * check if this queue family has support for presentation * /
                VkBool32 present_supported;
                MVK_VERIFY(vkGetPhysicalDeviceSurfaceSupportKHR(devices[i], j, vk_surface, &present_supported));

                if (vk_device.present_family_index < 0 && queue_families[j].queueCount > 0 && present_supported) {
                    vk_device.present_family_index = j;
                }
                if (vk_device.gfx_family_index < 0 && queue_families[j].queueCount > 0 && 
                    (queue_families[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)) 
                {
                    vk_device.gfx_family_index = j;
                }
                if (vk_device.transfer_family_index < 0 && queue_families[j].queueCount > 0 && 
                    !(queue_families[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
                     (queue_families[j].queueFlags & VK_QUEUE_TRANSFER_BIT))
                {
                    vk_device.transfer_family_index = j;
                }
            }
            mw_free(queue_families);

            // * accept only device that has support for presentation and drawing * /
            if (vk_device.present_family_index >= 0 && vk_device.gfx_family_index >= 0) {
                if (vk_device.transfer_family_index < 0)
                    vk_device.transfer_family_index = vk_device.gfx_family_index;

                vk_device.physical = devices[i];
                vk_device.properties = device_properties;
                vk_device.features = device_features;
                return;
            }
        }
    }
}

static bool_t select_physical_device(int32_t preferred_device_idx)
{
    uint32_t pd_count = 0;
    MVK_VERIFY(vkEnumeratePhysicalDevices(vk_instance, &pd_count, NULL));

    if (pd_count == 0) {
        mw_log_error("No Vulkan-capable devices found!");
        return MW_FALSE;
    }
    mw_log_debug("Found %d Vulkan-capable device%s", pd_count, pd_count == 1 ? "" : "s");

    VkPhysicalDevice *pds = (VkPhysicalDevice *)mw_malloc(pd_count * sizeof(VkPhysicalDevice));
    MVK_VERIFY(vkEnumeratePhysicalDevices(vk_instance, &pd_count, pds));

    get_best_physical_device(pds, preferred_device_idx < (int32_t)pd_count ? preferred_device_idx : -1, pd_count);
    mw_free(pds);

    if (vk_device.physical == VK_NULL_HANDLE) {
        mw_log_error("Could not find a suitable GPU!");
        return MW_FALSE;
    }

    uint32_t vk_major = VK_VERSION_MAJOR(vk_device.properties.apiVersion);
    uint32_t vk_minor = VK_VERSION_MINOR(vk_device.properties.apiVersion);
    uint32_t vk_patch = VK_VERSION_PATCH(vk_device.properties.apiVersion);
    uint32_t driver_major = VK_VERSION_MAJOR(vk_device.properties.driverVersion);
    uint32_t driver_minor = VK_VERSION_MINOR(vk_device.properties.driverVersion);
    uint32_t driver_patch = VK_VERSION_PATCH(vk_device.properties.driverVersion);

    mw_log_verbose("Physical Device Name:\t%s", vk_device.properties.deviceName);
    mw_log_verbose("    Vendor ID:       \t%d", vk_device.properties.vendorID);
    mw_log_verbose("    Device ID:       \t%d", vk_device.properties.deviceID);
    mw_log_verbose("    API Version:     \tv%d.%d.%d", vk_major, vk_minor, vk_patch);
    mw_log_verbose("    Driver Version:  \tv%d.%d.%d", driver_major, driver_minor, driver_patch);

    return MW_TRUE;
}

amw_result_t create_device(amw_vulkan_t *vk, int32_t preferred_device_idx)
{
    if (!select_physical_device(preferred_device_idx))
        return MW_FALSE;

    vk_config.vendor_name = vendor_name_string(vk_device.properties.vendorID);
    vk_config.device_type = device_type_string(vk_device.properties.deviceType);

    / * at least one queue (graphics and present combined) has to be present * /
    uint32_t queues_count = 1;
    float    queue_priority = 1.f;

    VkDeviceQueueCreateInfo queue_create_info[3];

    queue_create_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info[0].pNext = NULL;
    queue_create_info[0].flags = 0;
    queue_create_info[0].queueFamilyIndex = vk_device.gfx_family_index;
    queue_create_info[0].queueCount = 1;
    queue_create_info[0].pQueuePriorities = &queue_priority;

    queue_create_info[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info[1].pNext = NULL;
    queue_create_info[1].flags = 0;
    queue_create_info[1].queueFamilyIndex = 0;
    queue_create_info[1].queueCount = 1;
    queue_create_info[1].pQueuePriorities = &queue_priority;
    
    queue_create_info[2].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info[2].pNext = NULL;
    queue_create_info[2].flags = 0;
    queue_create_info[2].queueFamilyIndex = 0;
    queue_create_info[2].queueCount = 1;
    queue_create_info[2].pQueuePriorities = &queue_priority;

    VkPhysicalDeviceFeatures wanted_device_features = {
        .samplerAnisotropy = vk_device.features.samplerAnisotropy,
        .fillModeNonSolid  = vk_device.features.fillModeNonSolid, / * wireframe rendering * /
        .sampleRateShading = vk_device.features.sampleRateShading, / * sample shading * /
    };

    / * a graphics and present queue are different, two queues have to be created * /
    if (vk_device.gfx_family_index != vk_device.present_family_index) {
        queue_create_info[queues_count++].queueFamilyIndex = vk_device.present_family_index;
    }

    / * a separate transfer queue exists thats different from present and graphics queue? * /
    if (vk_device.transfer_family_index != vk_device.gfx_family_index && 
        vk_device.transfer_family_index != vk_device.present_family_index)
    {
        queue_create_info[queues_count++].queueFamilyIndex = vk_device.transfer_family_index;
    }

    const char *device_extensions[2] = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    int32_t enabled_ext_count = 1;

#ifdef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
	if (vk_config.vk_khr_portability_subset_available)
		device_extensions[enabled_ext_count++] = VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME;
#endif

    VkDeviceCreateInfo device_create_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pEnabledFeatures = &wanted_device_features,
        .ppEnabledExtensionNames = device_extensions,
        .enabledExtensionCount = enabled_ext_count,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = NULL,
        .queueCreateInfoCount = queues_count,
        .pQueueCreateInfos = queue_create_info
    };

#ifdef AMW_ENABLE_VALIDATION_LAYERS
#if VK_HEADER_VERSION > 101
	const char *validation_layers[] = { "VK_LAYER_KHRONOS_validation" };
#else
	const char *validation_layers[] = { "VK_LAYER_LUNARG_standard_validation" };
#endif
    if (validation_layers_enabled) {
        device_create_info.enabledLayerCount = amw_arraysize(validation_layers);
        device_create_info.ppEnabledLayerNames = validation_layers;
    }
#endif // * MW_DEBUG * /

    vkCreateDevice(vk_device.physical, &device_create_info, NULL, &vk_device.logical);

    mvk_load_device_pointers(vk_device.logical);

    vkGetDeviceQueue(vk_device.logical, vk_device.gfx_family_index, 0, &vk_device.gfx_queue);
    vkGetDeviceQueue(vk_device.logical, vk_device.present_family_index, 0, &vk_device.present_queue);
    vkGetDeviceQueue(vk_device.logical, vk_device.transfer_family_index, 0, &vk_device.transfer_queue);

    return MW_TRUE;
}
*/

int32_t amw_vk_init(amw_vulkan_t *vk, amw_window_t *window)
{
    amw_assert(vk);
    if (vk->initialized)
        return AMW_SUCCESS;

    amw_assert(window);

    int32_t res;

    amw_log_verbose("Initializing Vulkan rendering backend...");
    if (!amw_vk_open_driver()) {
        amw_log_error("Failed to open the Vulkan driver");
        return -1; /* error code */
    }

    /* enables the validation layers in debug mode, if supported */
    res = create_instance(vk);
    if (res != AMW_SUCCESS) {
        amw_log_error("Could not create a Vulkan instance");
        amw_vk_terminate(vk);
        return res;
    }

    AMW_VK_VERIFY(amw_vk_surface_create(vk->instance, window, NULL, &vk->surface));
    amw_hadal_framebuffer_size(window, &vk->width, &vk->height);

    //res = create_device(vk);
    if (res != AMW_SUCCESS) {
        amw_log_error("Could not create a Vulkan device");
        amw_vk_terminate(vk);
        return res;
    }

    amw_log_verbose("Vulkan backend initilialized");
    amw_arena_reset(&vk->temporary_arena);
    return AMW_SUCCESS;
}

void amw_vk_terminate(amw_vulkan_t *vk)
{
    amw_log_verbose("Terminating Vulkan...");
    if (vk == NULL)
        return;

    /* destroy swapchain,
     * descriptor pool, set layout,
     * buffer, free memory, 
     * pipeline, layout,
     * semaphores, fence,
     * command pool,
     * device,
     */

    if (vk->surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(vk->instance, vk->surface, NULL);

#ifdef AMW_BUILD_VALIDATION_LAYERS
    /* save to call even if layers are disabled */
    amw_vk_destroy_validation_layers(vk->instance);
#endif /* AMW_ENABLE_VALIDATION_LAYERS */

    if (vk->instance != VK_NULL_HANDLE)
        vkDestroyInstance(vk->instance, NULL);

    amw_vk_close_driver();
    amw_arena_free(&vk->temporary_arena);
}

void amw_vk_draw_frame(amw_vulkan_t *vk)
{
    (void)vk;
}

void amw_vk_framebuffer_resized_callback(void *data, amw_window_t *window, int32_t width, int32_t height)
{
    /* unused */
    (void)window;

    amw_vulkan_t *vk = data;

    vk->width = width;
    vk->height = height;
    vk->framebuffer_resized = AMW_TRUE;
}
