#pragma once

#include "core/asserts.h"
#include "defines.h"

#define VK_CHECK(exp)                                                                                                  \
    {                                                                                                                  \
        DASSERT(exp == VK_SUCCESS);                                                                                    \
    }

typedef struct vulkan_device
{
    VkPhysicalDevice physical;
    VkDevice         logical;

    // device specific stuff
    VkPhysicalDeviceProperties       properties;
    VkPhysicalDeviceFeatures         features;
    VkPhysicalDeviceMemoryProperties memory;

    // queue indicies

    i32 graphics_queue_index;
    i32 compute_queue_index;
    i32 transfer_queue_index;
    i32 present_queue_index;

    // queues
    VkQueue graphics_queue;
    VkQueue transfer_queue;
    VkQueue present_queue;

} vulkan_device;

typedef struct vulkan_context
{
    VkInstance            vk_instance;
    VkInstanceCreateInfo *instance_info;

#ifdef _DEBUG
    VkDebugUtilsMessengerEXT debug_messenger;
#endif
    // physical device and logical device
    vulkan_device vk_device;
    // vulkan surface
    VkSurfaceKHR vk_surface;

} vulkan_context;
