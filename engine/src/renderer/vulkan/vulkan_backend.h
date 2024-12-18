#pragma once

#include "renderer/renderer_backend.h"

b8 vulkan_initialize(struct renderer_backend *backend, const char *application_name, struct platform_state *platSate);

void vulkan_shutdown(struct renderer_backend *backend);

void vulkan_resize(struct renderer_backend *backend, u16 width, u16 height);

b8 vulkan_begin_frame(struct renderer_backend *backend, f32 delta_time);
b8 vulkan_end_frame(struct renderer_backend *backend, f32 delta_time);
