#pragma once
#include "defines.h"

typedef struct render_packet
{
    f32 delta_time;
} render_packet;

struct platform_state;

b8 renderer_initialize(const char *application_name, struct platform_state *plat_state);

void renderer_shutdown();

void renderer_on_resized(u16 width, u16 height);

b8 renderer_draw_frame(render_packet *packet);
