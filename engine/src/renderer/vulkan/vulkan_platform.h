#pragma once

#include "defines.h"

struct vulkan_context;
struct platform_state;

b8 platform_create_vk_surface(struct platform_state *state, struct vulkan_context *context);

void platform_get_specific_surface_extensions(const char ***array);
