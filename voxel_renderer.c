#include "voxel_renderer.h"

bool voxel_renderer_init(voxel_renderer *renderer, dm_context *context, dm_handle resource_heap, dm_handle sampler_heap)
{
    dm_raster_pipe_desc pipe_desc = {
    
    };

    return true;
}

void voxel_renderer_render(voxel_renderer *renderer, dm_context *context, dm_handle swapchain)
{
    dm_render_command_begin_rendering(context, swapchain, 0.1f, 0.2f, 0.2f, 1.f, 1.f);

        //dm_render_command_bind_pipeline(context, renderer->pipeline);
        //dm_render_command_bind_index_buffer(context, renderer->ib_gpu, 0);
        //dm_render_command_push_data(context, &renderer->push_data, sizeof(renderer->push_data));
        //dm_render_command_draw(context, 6, 1);
    
    dm_render_command_end_rendering(context, swapchain);
}
