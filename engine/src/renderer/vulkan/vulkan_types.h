#pragma once

#include "core/asserts.h"
#include "defines.h"

#include <vulkan/vulkan.h>

#define VK_CHECK(exp)                                                                                                                                \
    {                                                                                                                                                \
        DASSERT(exp == VK_SUCCESS);                                                                                                                  \
    }

typedef struct vulkan_swapchain_support_info
{
    u32 format_count;
    u32 present_mode_count;

    VkSurfaceCapabilitiesKHR surface_capabilites;
    VkSurfaceFormatKHR      *formats;
    VkPresentModeKHR        *present_modes;
} vulkan_swapchain_support_info;

typedef struct vulkan_device
{
    VkPhysicalDevice              physical;
    VkDevice                      logical;
    vulkan_swapchain_support_info swapchain_info;

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
    VkInstance vk_instance;

#ifdef _DEBUG
    VkDebugUtilsMessengerEXT debug_messenger;
#endif
    // physical device and logical device
    vulkan_device vk_device;
    // vulkan surface
    VkSurfaceKHR vk_surface;

} vulkan_context;
