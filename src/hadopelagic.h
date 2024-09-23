#ifndef _amw_hadopelagic_h_
#define _amw_hadopelagic_h_

#include "amw.h"

#include "renderer/vulkan.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct amw_window amw_window_t;

int32_t       amw_hadal_init(int32_t platform_id);

amw_window_t *amw_window_create(const char *title, int32_t width, int32_t height, uint32_t flags);
void          amw_window_destroy(amw_window_t *window);
VkResult      amw_vk_surface_create(amw_window_t *window, VkInstance instance, const VkAllocationCallbacks *allocator, VkSurfaceKHR *surface);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _amw_hadopelagic_h_ */
