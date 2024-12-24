#include "vulkan_command_buffer.h"

void vk_command_buffer_allocate(vulkan_context *context, VkCommandPool pool, vulkan_command_buffer *out_command_buffer)
{
    VkCommandBufferAllocateInfo command_buffer_info = {};

    command_buffer_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    command_buffer_info.pNext              = 0;
    command_buffer_info.commandPool        = pool;
    command_buffer_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    command_buffer_info.commandBufferCount = 1;

    out_command_buffer->state = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
    VK_CHECK(vkAllocateCommandBuffers(context->vk_device.logical, &command_buffer_info, &out_command_buffer->handle));
    out_command_buffer->state = COMMAND_BUFFER_STATE_READY;
}

void vk_command_buffer_free(vulkan_context *context, VkCommandPool pool, vulkan_command_buffer *out_command_buffer)
{
    vkFreeCommandBuffers(context->vk_device.logical, pool, 1, &out_command_buffer->handle);
    out_command_buffer->handle = 0;
    out_command_buffer->state  = COMMAND_BUFFER_STATE_NOT_ALLOCATED;
}

void vk_command_buffer_begin(vulkan_command_buffer *command_buffer)
{
    VkCommandBufferBeginInfo buffer_begin_info = {};

    buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    buffer_begin_info.pNext = 0;
    // make this configurable
    buffer_begin_info.flags = 0;

    buffer_begin_info.pInheritanceInfo = 0;

    VK_CHECK(vkBeginCommandBuffer(command_buffer->handle, &buffer_begin_info));
    command_buffer->state = COMMAND_BUFFER_STATE_RECORDING;
}

void vulkanCommandBufferEnd(vulkan_command_buffer *command_buffer)
{
    VK_CHECK(vkEndCommandBuffer(command_buffer->handle));
    command_buffer->state = COMMAND_BUFFER_STATE_RECORDING_ENDED;
}
void vulkanCommandBufferUpdateSubmitted(vulkan_command_buffer *command_buffer)
{
    command_buffer->state = COMMAND_BUFFER_STATE_SUBMITTED;
}
void vulkanCommandBufferReset(vulkan_command_buffer *command_buffer)
{
    command_buffer->state = COMMAND_BUFFER_STATE_READY;
}
