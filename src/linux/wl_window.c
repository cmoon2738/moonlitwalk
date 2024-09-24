#include "../internal.h"

bool amw_wayland_window_create(amw_window_t *window)
{
    return 0;
}

void amw_wayland_window_destroy(amw_window_t *window)
{

}

bool amw_wayland_vk_physical_device_presentation_support(VkInstance instance, VkPhysicalDevice device, uint32_t queue_family)
{
    return 0;
}

VkResult amw_wayland_vk_surface_create(VkInstance instance, 
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

    return res;
}
