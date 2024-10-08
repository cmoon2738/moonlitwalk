#include "../core/hadal.h"
#include "wayland.h"

#ifdef AMW_NATIVE_VULKAN
bool hadal_wayland_physical_device_presentation_support(VkPhysicalDevice device, uint32_t queue_family)
{
    return vkGetPhysicalDeviceWaylandPresentationSupportKHR(device, queue_family, hadal.wl.display);
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
        amw_log_error("Wayland failed to create a Vulkan surface: %s", amw_vkresult(res));
    }

    amw_log_verbose("Created a Wayland VkSurface");
    return res;
}
#endif /* AMW_NATIVE_VULKAN */

