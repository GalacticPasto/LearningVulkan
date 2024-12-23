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

    VkSurfaceCapabilitiesKHR surface_capabilities;
    VkSurfaceFormatKHR      *formats;
    VkPresentModeKHR        *present_modes;
} vulkan_swapchain_support_info;

typedef struct vk_image
{
    VkImage handle;
} vk_image;

typedef struct vk_swapchain
{
    VkSwapchainKHR handle;
    VkFormat       image_format;
    VkExtent2D     extent;
    u32            image_count;
    VkImage       *images;
    VkImageView   *image_views;
} vk_swapchain;

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
    u16 frame_buffer_width;
    u16 frame_buffer_height;

    VkInstance vk_instance;
#ifdef _DEBUG
    VkDebugUtilsMessengerEXT debug_messenger;
#endif
    // physical device and logical device
    vulkan_device vk_device;
    // vulkan surface
    VkSurfaceKHR vk_surface;

    vk_swapchain vk_swapchain;
    u32          image_index; // to track the current VkImage
    u32          current_frame;

    b8 recreating_swapchain;
} vulkan_context;
