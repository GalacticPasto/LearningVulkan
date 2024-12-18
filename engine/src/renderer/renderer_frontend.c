#include "renderer_frontend.h"
#include "core/dmemory.h"
#include "core/logger.h"
#include "renderer_backend.h"

static renderer_backend *backend = 0;

b8 renderer_initialize(const char *application_name, struct platform_state *plat_state)
{

    backend               = dallocate(sizeof(renderer_backend), MEMORY_TAG_RENDERER);
    backend->frame_number = 0;

    b8 result = renderer_backend_create(plat_state, backend);

    if (!backend->initialize(backend, application_name, plat_state))
    {
        DERROR("Failed to initialized renderer backend!!! Shutting down");
        return false;
    }

    return true;
}

void renderer_shutdown()
{
    backend->shutdown(backend);
    dfree(backend, sizeof(renderer_backend), MEMORY_TAG_RENDERER);
}

void renderer_on_resized(u16 width, u16 height)
{
    if (backend->resized)
    {
        backend->resized(backend, width, height);
    }
    else
    {
        DWARN("the backend doesnt supoort resize events yet");
    }
}

b8 renderer_draw_frame(render_packet *packet)
{
    if (backend->begin_frame(backend, packet->delta_time))
    {

        b8 result = backend->end_frame(backend, packet->delta_time);
        backend->frame_number++;
        if (!result)
        {
            DERROR("rendererEndFrame failed. Application shutting down...");
            return false;
        }
    }

    return true;
}
