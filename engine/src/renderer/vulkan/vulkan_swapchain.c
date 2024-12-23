#include "vulkan_swapchain.h"
#include "core/dmemory.h"

b8 vk_create_swapchain(vulkan_context *context, u32 width, u32 height)
{

    // swap surface format
    vulkan_swapchain_support_info *swapchain_support_info = &context->vk_device.swapchain_info;

    // chose the best format
    VkSurfaceFormatKHR *best_format = 0;

    for (i32 i = 0; i < swapchain_support_info->format_count; i++)
    {
        if (swapchain_support_info->formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
            swapchain_support_info->formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
        {
            best_format = &swapchain_support_info->formats[i];
            break;
        }
    }
    if (!best_format)
    {
        best_format = &swapchain_support_info->formats[0];
    }

    // swapchain extent
    VkExtent2D swapchain_extent = {width, height};

    if (swapchain_support_info->surface_capabilities.currentExtent.width != UINT32_MAX)
    {
        swapchain_extent = swapchain_support_info->surface_capabilities.currentExtent;
    }

    // Clamp the value allowed by the GPU. // idk why we do this tbh
    VkExtent2D min = swapchain_support_info->surface_capabilities.minImageExtent;
    VkExtent2D max = swapchain_support_info->surface_capabilities.maxImageExtent;

    swapchain_extent.width  = DCLAMP(swapchain_extent.width, min.width, max.width);
    swapchain_extent.height = DCLAMP(swapchain_extent.height, min.height, max.height);

    // present Modes
    VkPresentModeKHR present_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (u32 i = 0; i < swapchain_support_info->present_mode_count; ++i)
    {
        VkPresentModeKHR mode = swapchain_support_info->present_modes[i];
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            present_mode = mode;
            break;
        }
    }

    // image count
    u32 image_count = swapchain_support_info->surface_capabilities.minImageCount + 1;
    // to make sure that the image count is less than the max image count supported by the gpu
    if (swapchain_support_info->surface_capabilities.maxImageCount > 0 && image_count > swapchain_support_info->surface_capabilities.maxImageCount)
    {
        image_count = swapchain_support_info->surface_capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapchain_info = {};
    swapchain_info.sType                    = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_info.pNext                    = 0;
    swapchain_info.flags                    = 0;
    swapchain_info.surface                  = context->vk_surface;
    swapchain_info.minImageCount            = image_count;
    swapchain_info.imageFormat              = best_format->format;
    swapchain_info.imageColorSpace          = best_format->colorSpace;
    swapchain_info.imageExtent              = swapchain_extent;
    swapchain_info.imageArrayLayers         = 1;
    swapchain_info.imageUsage               = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // check which image sharing mode to use

    if (context->vk_device.graphics_queue_index != context->vk_device.present_queue_index)
    {
        u32 queue_family_indicies[] = {(u32)context->vk_device.graphics_queue_index, (u32)context->vk_device.present_queue_index};

        swapchain_info.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
        swapchain_info.queueFamilyIndexCount = 2;
        swapchain_info.pQueueFamilyIndices   = queue_family_indicies;
    }
    else
    {
        swapchain_info.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_info.queueFamilyIndexCount = 0;
        swapchain_info.pQueueFamilyIndices   = 0;
    }

    swapchain_info.preTransform   = swapchain_support_info->surface_capabilities.currentTransform;
    swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_info.presentMode    = present_mode;
    swapchain_info.clipped        = VK_TRUE;
    swapchain_info.oldSwapchain   = VK_NULL_HANDLE;

    VK_CHECK(vkCreateSwapchainKHR(context->vk_device.logical, &swapchain_info, 0, &context->vk_swapchain.handle));

    context->vk_swapchain.image_format = best_format->format;
    context->vk_swapchain.extent       = swapchain_extent;

    // start the current frame
    context->current_frame = 0;

    context->vk_swapchain.image_count = 0;
    VK_CHECK(vkGetSwapchainImagesKHR(context->vk_device.logical, context->vk_swapchain.handle, &context->vk_swapchain.image_count, 0));

    if (!context->vk_swapchain.images)
    {
        context->vk_swapchain.images = (VkImage *)dallocate(sizeof(VkImage) * context->vk_swapchain.image_count, MEMORY_TAG_RENDERER);
    }
    if (!context->vk_swapchain.image_views)
    {
        context->vk_swapchain.image_views = (VkImageView *)dallocate(sizeof(VkImageView) * context->vk_swapchain.image_count, MEMORY_TAG_RENDERER);
    }
    VK_CHECK(vkGetSwapchainImagesKHR(context->vk_device.logical, context->vk_swapchain.handle, &context->vk_swapchain.image_count,
                                     context->vk_swapchain.images));

    for (i32 i = 0; i < context->vk_swapchain.image_count; i++)
    {
        VkImageViewCreateInfo image_info = {};

        image_info.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_info.pNext                           = 0;
        image_info.flags                           = 0;
        image_info.image                           = context->vk_swapchain.images[i];
        image_info.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
        image_info.format                          = context->vk_swapchain.image_format;
        image_info.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_info.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_info.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_info.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
        image_info.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        image_info.subresourceRange.baseMipLevel   = 0;
        image_info.subresourceRange.levelCount     = 1;
        image_info.subresourceRange.baseArrayLayer = 0;
        image_info.subresourceRange.layerCount     = 1;

        VK_CHECK(vkCreateImageView(context->vk_device.logical, &image_info, 0, &context->vk_swapchain.image_views[i]));
    }

    return true;
}

b8 vk_destroy_swapchain(vulkan_context *context)
{
    // destroy image views
    for (i32 i = 0; i < context->vk_swapchain.image_count; i++)
    {
        vkDestroyImageView(context->vk_device.logical, context->vk_swapchain.image_views[i], 0);
    }
    if (context->vk_swapchain.image_views)
    {
        dfree(context->vk_swapchain.image_views, sizeof(VkImageView) * context->vk_swapchain.image_count, MEMORY_TAG_RENDERER);
    }
    if (context->vk_swapchain.images)
    {
        dfree(context->vk_swapchain.images, sizeof(VkImage) * context->vk_swapchain.image_count, MEMORY_TAG_RENDERER);
    }
    vkDestroySwapchainKHR(context->vk_device.logical, context->vk_swapchain.handle, 0);
    return true;
}
