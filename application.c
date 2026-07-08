#include "application.h"

bool application_init(application *app, size_t size, u16 width, u16 height, const char *title)
{
    dm_arena_create(&app->arena, size);
    if(!app->arena.start) return false;

    app->context = dm_arena_alloc(&app->arena, sizeof(dm_context), &app->context_offset);
    if(!app->context) return false;

    if(!dm_init(app->context, width, height, title, 0)) return false;
#ifdef DM_DEBUG
    LOG_DEBUG("DM arena capacity :  %zu", app->context->arena.capacity);
    LOG_DEBUG("DM arena size     :  %zu", app->context->arena.size);
    LOG_DEBUG("DM arena remaining:  %zu", app->context->arena.capacity - app->context->arena.size);
#endif

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

    /*****************
     * VOXEL RENDERER
     ******************/
    app->renderer = dm_arena_alloc(&app->arena, sizeof(voxel_renderer), &app->renderer_offset);
    if(!app->renderer) return false;
    if(!voxel_renderer_init(app->renderer, app->context, &app->arena)) return false;
#ifdef DM_DEBUG
    LOG_DEBUG("App arena capacity:   %zu", app->arena.capacity);
    LOG_DEBUG("App arena size:       %zu", app->arena.size);
    LOG_DEBUG("App arena remaining:  %zu", app->arena.capacity - app->arena.size);
#endif

    return true;
}

void application_run(application *app)
{
    while(dm_is_running(app->context))
    {
        /**************
         * BEGIN FRAME
         ***************/
        if(!dm_update_begin(app->context)) break;

        if(!voxel_renderer_update(app->renderer, app->context)) break;

        /*********
         * RENDER
         **********/
        if(!dm_render_begin(app->context)) break;

        voxel_renderer_render(app->renderer, app->context, app->swapchain);

        if(!dm_render_end(app->context))   break;

        /************
         * END FRAME
         *************/
        dm_update_end(app->context);
    }
}

void application_shutdown(application *app)
{
    dm_shutdown(app->context);
    dm_arena_detroy(&app->arena);
}
