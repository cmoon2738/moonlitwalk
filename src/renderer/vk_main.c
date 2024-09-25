#include <moonlitwalk/vk.h>
#include <moonlitwalk/hadopelagic.h>

#ifdef AMW_ENABLE_VALIDATION_LAYERS
static bool validation_layers_enabled = AMW_TRUE;
static const char *validation_layers[] = {
    "VK_LAYER_KHRONOS_validation"
};
#endif /* AMW_ENABLE_VALIDATION_LAYERS */

const char *device_extensions[] = {
    "VK_KHR_swapchain",
    "VK_KHR_surface",
    "VK_KHR_dynamic_rendering",
#ifdef AMW_ENABLE_VALIDATION_LAYERS
    "VK_KHR_shader_non_semantic_info",
#endif /* AMW_ENABLE_VALIDATION_LAYERS */
};

/* cube demo */
const vertex_t cube_vertices[] = {
    { .pos = {  0.0f,  0.0f }, .color = { 0.25f, 0.25f, 0.25f } },
    { .pos = { -0.5f, -0.5f }, .color = { 1.00f, 0.00f, 0.25f } },
    { .pos = {  0.5f, -0.5f }, .color = { 0.76f, 0.25f, 0.25f } },
    { .pos = {  0.5f,  0.5f }, .color = { 0.05f, 0.50f, 0.25f } },
    { .pos = { -0.5f,  0.5f }, .color = { 0.25f, 0.75f, 0.25f } },
};

const index_t cube_indices[] = {
    0, 1, 2,
    0, 2, 3,
    0, 3, 4,
    0, 4, 1
};

VkVertexInputBindingDescription vertex_binding_descriptions[] = {
    {
        .binding = 0,
        .stride = sizeof(vertex_t),
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    },
};

VkVertexInputAttributeDescription vertex_attribute_descriptions[] = {
    {
        .binding = 0,
        .location = 0,
        .format = VK_FORMAT_R32G32_SFLOAT,
        .offset = offsetof(vertex_t, pos),
    },
    {
        .binding = 0,
        .location = 1,
        .format = VK_FORMAT_R32G32B32_SFLOAT,
        .offset = offsetof(vertex_t, color),
    },
};

bool amw_vk_init(amw_vulkan_t *vk, amw_window_t *window)
{
    amw_assert(vk);
    amw_assert(!vk->initialized);
    amw_assert(window);

    /* TODO */

    return AMW_TRUE;
}

void amw_vk_terminate(amw_vulkan_t *vk)
{
    if (!vk || !vk->initialized) {
        return;
    }

    /* destroy swapchain,
     * descriptor pool, set layout,
     * buffer, free memory, 
     * pipeline, layout,
     * semaphores, fence,
     * command pool,
     * device,
     * surface,
     */

#ifdef AMW_ENABLE_VALIDATION_LAYERS
    if (validation_layers_enabled)
        amw_vk_destroy_validation_layers(vk->instance);
#endif /* AMW_ENABLE_VALIDATION_LAYERS */

    /* destroy instance */
    amw_vk_close_library();
    amw_arena_free(&vk->swapchain_arena);
}

void amw_vk_draw_frame(amw_vulkan_t *vk)
{

}

void amw_vk_framebuffer_resized_callback(void *data, struct amw_window *window, int32_t width, int32_t height)
{
    (void)window;
    amw_vulkan_t *vk = data;

    vk->width = width;
    vk->height = height;
    vk->framebuffer_resized = AMW_TRUE;
}
