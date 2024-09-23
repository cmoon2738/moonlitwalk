#include "vulkan.h"

static VkDebugUtilsMessengerEXT validation_messenger = VK_NULL_HANDLE;

#ifdef AMW_DEBUG
static bool enable_validation = AMW_TRUE;
#else
static bool enable_validation = AMW_FALSE;
#endif

/* layer message to string */
static const char* msg_to_string(VkDebugUtilsMessageTypeFlagsEXT type)
{
	int g = (type & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) != 0 ? 1 : 0;
	int p = (type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) != 0 ? 1 : 0;
	int v = (type & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) != 0 ? 1 : 0;

	if (g) return "";
	if (p && !v) return "(performance)";
	if (p &&  v) return "(performance and validation)";
	if (v) return "(validation)";

	return "?";
}

/* validation layer callback function (VK_EXT_debug_utils) */
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_utils_callback(VkDebugUtilsMessageSeverityFlagBitsEXT msg_severity,
													  	   VkDebugUtilsMessageTypeFlagsEXT msg_type,
														   const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
														   void *userdata)
{
    (void)userdata;
	switch (msg_severity) {
#if AMW_LOG_USE_COLOR
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        amw_log_info("\x1b[32m%s \x1b[0m%s", callback_data->pMessage, msg_to_string(msg_type));
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        amw_log_verbose("\x1b[94m%s \x1b[0m%s", callback_data->pMessage, msg_to_string(msg_type));
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        amw_log_warn("\x1b[33m%s \x1b[0m%s", callback_data->pMessage, msg_to_string(msg_type));
		break;
	default:
        amw_log_error("\x1b[31m%s \x1b[0m%s", callback_data->pMessage, msg_to_string(msg_type));
#else
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        amw_log_info("%s %s", callback_data->pMessage, msg_to_string(msg_type));
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        amw_log_verbose("%s %s", callback_data->pMessage, msg_to_string(msg_type));
		break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        amw_log_warn("%s %s", callback_data->pMessage, msg_to_string(msg_type));
		break;
	default:
        amw_log_error("%s %s", callback_data->pMessage, msg_to_string(msg_type));
#endif /* MW_LOG_USE_COLOR */
		amw_assert(!"vulkan validation error");
	}
	return VK_FALSE;
}

void amw_vk_create_validation_layers(VkInstance instance)
{
    if (!enable_validation)
        return;

    VkDebugUtilsMessengerCreateInfoEXT callback_info = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = NULL,
        .flags = 0,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debug_utils_callback,
        .pUserData = NULL
    };
    AMW_VK_VERIFY(vkCreateDebugUtilsMessengerEXT(instance, &callback_info, NULL, &validation_messenger));
    amw_log_debug("Vulkan validation layers enabled");
}

void amw_vk_destroy_validation_layers(VkInstance instance)
{
    if (!enable_validation)
        return;

    if (validation_messenger != VK_NULL_HANDLE) {
        vkDestroyDebugUtilsMessengerEXT(instance, validation_messenger, NULL);
        validation_messenger = VK_NULL_HANDLE;
    }
    amw_log_debug("Vulkan validation layers destroyed");
}
