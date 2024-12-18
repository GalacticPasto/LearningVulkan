#include "renderer_backend.h"
#include "vulkan/vulkan_backend.h"

b8 renderer_backend_create(struct platform_state *plat_state, renderer_backend *out_renderer_backend)
{
    // create vulkan backend
    out_renderer_backend->plat_state  = plat_state;
    out_renderer_backend->initialize  = vulkan_initialize;
    out_renderer_backend->shutdown    = vulkan_shutdown;
    out_renderer_backend->resized     = vulkan_resize;
    out_renderer_backend->begin_frame = vulkan_begin_frame;
    out_renderer_backend->end_frame   = vulkan_end_frame;

    return true;
}
void renderer_backend_shutdown(renderer_backend *renderer_backend)
{
    renderer_backend->plat_state  = 0;
    renderer_backend->initialize  = 0;
    renderer_backend->shutdown    = 0;
    renderer_backend->resized     = 0;
    renderer_backend->begin_frame = 0;
    renderer_backend->end_frame   = 0;
}
