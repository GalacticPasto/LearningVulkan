#include "vulkan_backend.h"
#include "core/logger.h"
#include <vulkan/vulkan.h>

b8 vulkan_initialize(struct renderer_backend *backend, const char *application_name, struct platform_state *platSate)
{

    // typedef VkResult (VKAPI_PTR *PFN_vkCreateInstance)(const VkInstanceCreateInfo* pCreateInfo, const
    // VkAllocationCallbacks* pAllocator, VkInstance* pInstance);
    //  initialize a vk instance
    VkApplicationInfo app_info  = {};
    app_info.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName   = application_name;
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName        = "learning_vulkan";
    app_info.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion         = VK_API_VERSION_1_0;

    // layers

    u32 layer_count = 0;

    VK_CHECK(vkEnumerateInstanceLayerProperties(&layer_count, 0));
    DINFO("available layer counts %d", layer_count);

    VkInstanceCreateInfo instance_info = {};

    instance_info.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_info.pNext            = 0;
    instance_info.flags            = 0;
    instance_info.pApplicationInfo = &app_info;
    instance_info.enabledLayerCount;
    instance_info.ppEnabledLayerNames;
    instance_info.enabledExtensionCount;
    instance_info.ppEnabledExtensionNames;

    return true;
}

void vulkan_shutdown(struct renderer_backend *backend)
{
}

void vulkan_resize(struct renderer_backend *backend, u16 width, u16 height)
{
}

b8 vulkan_begin_frame(struct renderer_backend *backend, f32 delta_time)
{

    return false;
}

b8 vulkan_end_frame(struct renderer_backend *backend, f32 delta_time)
{
    return false;
}
