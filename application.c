#include "application.h"

bool application_init(application *app, size_t size, u16 width, u16 height, const char *title)
{
    dm_arena_create(&app->arena, size);
    if(!app->arena.start) return false;

    app->context = dm_arena_alloc(&app->arena, sizeof(dm_context), &app->context_offset);
    if(!app->context) return false;

    if(!dm_init(app->context, width, height, title, 0)) return false;

    /*****************
     * RENDER HANDLES
     ******************/
    dm_render_attachment_desc color_desc = {
        .load_op=DM_RENDER_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op=DM_RENDER_ATTACHMENT_STORE_OP_STORE
    };
    dm_render_attachment_desc depth_desc = {
        .load_op=DM_RENDER_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op=DM_RENDER_ATTACHMENT_STORE_OP_DONT_CARE
    };
    dm_render_target_desc swapchain_desc = {
        .type=DM_RENDER_TARGET_TYPE_SWAPCHAIN,
        .color_attachment=color_desc,
        .depth_attachment=depth_desc
    };
    if(!dm_renderer_create_render_target(app->context, swapchain_desc, &app->swapchain)) return false;

    dm_resource_descriptor_heap_desc resource_heap_desc = {
        .buffer_count=DM_MAX_BUFFERS,
        .texture_count=DM_MAX_TEXTURES
    };
    dm_sampler_descriptor_heap_desc sampler_heap_desc = {
        .sampler_count=DM_MAX_SAMPLERS
    };

    if(!dm_renderer_create_resource_descriptor_heap(app->context, resource_heap_desc, &app->resource_heap)) return false;
    if(!dm_renderer_create_sampler_descriptor_heap(app->context, sampler_heap_desc, &app->sampler_heap)) return false;

    /*****************
     * VOXEL RENDERER
     ******************/
    app->renderer = dm_arena_alloc(&app->arena, sizeof(voxel_renderer), &app->renderer_offset);
    if(!app->renderer) return false;
    if(!voxel_renderer_init(app->renderer, app->context, &app->arena, app->resource_heap, app->sampler_heap)) return false;

    return true;
}

void application_run(application *app)
{
    while(dm_is_running(app->context))
    {
        dm_update(app->context);

        /*********
         * RENDER
         **********/
        if(!dm_render_begin(app->context)) break;

        dm_render_command_bind_resource_descriptor_heap(app->context, app->resource_heap);
        dm_render_command_bind_sampler_descriptor_heap(app->context, app->sampler_heap);

        //dm_render_command_begin_rendering(app->context, app->swapchain, 0.5f, 0.5f, 0.1f, 1.f, 1.f);
            voxel_renderer_render(app->renderer, app->context, app->swapchain);
        //dm_render_command_end_rendering(app->context, app->swapchain);

        if(!dm_render_end(app->context))   break;
    }
}

void application_shutdown(application *app)
{
    dm_shutdown(app->context);
    dm_arena_detroy(&app->arena);
}
