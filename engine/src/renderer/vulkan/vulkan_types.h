#pragma once

#include "core/asserts.h"
#include "defines.h"
#include <vulkan/vulkan.h>

#define VK_CHECK(exp)                                                                                                  \
    {                                                                                                                  \
        DASSERT(exp == VK_SUCCESS);                                                                                    \
    }

typedef struct vulkan_context
{
    VkInstance            vk_instance;
    VkInstanceCreateInfo *instance_info;

#ifdef _DEBUG
    VkDebugUtilsMessengerEXT debug_messenger;
#endif

} vulkan_context;
