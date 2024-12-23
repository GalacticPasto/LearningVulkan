#pragma once

#include "defines.h"
#include "vulkan_types.h"

b8 vk_create_swapchain(vulkan_context *context, u32 width, u32 height);
b8 vk_destroy_swapchain(vulkan_context *context);
