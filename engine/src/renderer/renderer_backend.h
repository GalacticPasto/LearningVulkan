#pragma once

#include "defines.h"

typedef struct renderer_backend
{
    struct platform_state *plat_state;

    u64 frame_number;

    b8 (*initialize)(struct renderer_backend *backend, const char *application_name, struct platform_state *platSate);

    void (*shutdown)(struct renderer_backend *backend);

    void (*resized)(struct renderer_backend *backend, u16 width, u16 height);

    b8 (*begin_frame)(struct renderer_backend *backend, f32 delta_time);
    b8 (*end_frame)(struct renderer_backend *backend, f32 delta_time);

} renderer_backend;

b8   renderer_backend_create(struct platform_state *platSate, renderer_backend *out_renderer_backend);
void renderer_backend_shutdown(renderer_backend *renderer_backend);
