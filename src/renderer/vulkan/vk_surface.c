#include "../../hadopelagic.h"

char *amw_vk_hadal_surface_extension(void)
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

#ifdef AMW_PLATFORM_WINDOWS
#endif /* AMW_PLATFORM_WINDOWS */

#ifdef AMW_PLATFORM_APPLE /* OSX / iOS */
#endif /* AMW_PLATFORM_APPLE */

#ifdef AMW_PLATFORM_ANDROID
#endif /* AMW_PLATFORM_ANDROID */

#ifdef AMW_NATIVE_WAYLAND
bool hadal_wayland_physical_device_presentation_support(VkPhysicalDevice device, uint32_t queue_family)
{
    return vkGetPhysicalDeviceWaylandPresentationSupportKHR ?
        vkGetPhysicalDeviceWaylandPresentationSupportKHR(device, queue_family, hadal.wl.display) : AMW_FALSE;
}
VkResult hadal_wayland_create_surface(VkInstance instance, 
                                       amw_window_t *window, 
                                       const VkAllocationCallbacks *allocator, 
                                       VkSurfaceKHR *surface)
{
    VkResult res;
    VkWaylandSurfaceCreateInfoKHR wl_sc_info;

    amw_zero(wl_sc_info);
    wl_sc_info.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    wl_sc_info.display = hadal.wl.display;
    wl_sc_info.surface = window->wl.surface;

    res = vkCreateWaylandSurfaceKHR(instance, &wl_sc_info, allocator, surface);
    if (res) {
        amw_log_error("Wayland failed to create a Vulkan surface: %s", amw_vk_result(res));
    }

    amw_log_verbose("Created a Wayland surface");
    return res;
}
#endif /* AMW_NATIVE_WAYLAND */

#ifdef AMW_NATIVE_XCB
#endif /* AMW_NATIVE_XCB */

#ifdef AMW_NATIVE_KMS
#endif /* AMW_NATIVE_KMS */

// TODO headless surface
