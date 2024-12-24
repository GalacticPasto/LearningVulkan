#include "vulkan_renderpass.h"

b8 vk_create_renderpass(vulkan_context *context)
{
    VkAttachmentDescription color_attachment = {};

    color_attachment.flags          = 0;
    color_attachment.format         = context->vk_swapchain.image_format;
    color_attachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    color_attachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    color_attachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    color_attachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_reference = {};
    color_attachment_reference.attachment            = 0;
    color_attachment_reference.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;

    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments    = &color_attachment_reference;

    VkRenderPassCreateInfo renderpass_info = {};

    renderpass_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderpass_info.pNext           = 0;
    renderpass_info.flags           = 0;
    renderpass_info.attachmentCount = 1;
    renderpass_info.pAttachments    = &color_attachment;
    renderpass_info.subpassCount    = 1;
    renderpass_info.pSubpasses      = &subpass;
    renderpass_info.dependencyCount = 0;
    renderpass_info.pDependencies   = 0;

    VK_CHECK(vkCreateRenderPass(context->vk_device.logical, &renderpass_info, 0, &context->vk_renderpass.handle));

    return true;
}

b8 vk_destroy_renderpass(vulkan_context *context)
{
    vkDestroyRenderPass(context->vk_device.logical, context->vk_renderpass.handle, 0);
    return true;
}
