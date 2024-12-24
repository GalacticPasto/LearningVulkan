#pragma once

#include "vulkan_types.h"

void vk_command_buffer_allocate(vulkan_context *context, VkCommandPool pool, vulkan_command_buffer *out_command_buffer);
void vk_command_buffer_free(vulkan_context *context, VkCommandPool pool, vulkan_command_buffer *out_command_buffer);

void vk_command_buffer_begin(vulkan_command_buffer *command_buffer);
void vulkanCommandBufferReset(vulkan_command_buffer *command_buffer);
void vulkanCommandBufferUpdateSubmitted(vulkan_command_buffer *command_buffer);
void vulkanCommandBufferEnd(vulkan_command_buffer *command_buffer);
