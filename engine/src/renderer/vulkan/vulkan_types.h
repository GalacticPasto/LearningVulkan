#pragma once

#include "core/asserts.h"
#include "defines.h"
#include <vulkan/vulkan.h>

#define VK_CHECK(exp)                                                                                                  \
    {                                                                                                                  \
        DASSERT(exp == VK_SUCCESS);                                                                                    \
    }

typedef struct vulkan_device
{
    VkPhysicalDevice physical_device;

    // device specific stuff
    VkPhysicalDeviceProperties       properties;
    VkPhysicalDeviceFeatures         features;
    VkPhysicalDeviceMemoryProperties memory;

    // queue indicies

    i32 graphics_queue_index;
    i32 compute_queue_index;
    i32 transfer_queue_index;

} vulkan_device;

typedef struct vulkan_context
{
    VkInstance            vk_instance;
    VkInstanceCreateInfo *instance_info;

#ifdef _DEBUG
    VkDebugUtilsMessengerEXT debug_messenger;
#endif
    // physical device

    vulkan_device device;

} vulkan_context;
