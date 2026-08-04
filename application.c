#include "application.h"
#include "random_gen.h"

bool application_init(application *app, u16 width, u16 height, const char *title)
{
    random_init();

    const size_t size = sizeof(instance_data);

    dm_arena_create(&app->arena, size);

    /*************
     * DARKMATTER
     **************/
    if(!dm_init(&app->context, width, height, title, 0)) return false;

    /***********
     * RENDERER
     ************/
    if(!renderer_init(&app->renderer, &app->context)) return false;

    /********
     * IMGUI 
     *********/
    if(!gui_init(&app->context, &app->gui_ctx)) return false;

    /************
     * INSTANCES 
     *************/
    app->instances = dm_arena_alloc(&app->arena, sizeof(instance_data));
    instances_init(app->instances);

    // submit resources
    u32 resource_count = 0;

    dm_resource *resources[100] = { 0 };

    // buffers
    resources[resource_count++] = &app->renderer.vb;
    resources[resource_count++] = &app->renderer.ib;
    resources[resource_count++] = &app->renderer.quad_ib;

    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        resources[resource_count++] = &app->renderer.cb[i];
        resources[resource_count++] = &app->renderer.instb[i];
        resources[resource_count++] = &app->renderer.compute_frame_data[i];

        resources[resource_count++] = &app->gui_ctx.resources.vb[i];
        resources[resource_count++] = &app->gui_ctx.resources.ib[i];
        resources[resource_count++] = &app->gui_ctx.resources.scene[i];
    }

    // textures
    resources[resource_count++] = &app->renderer.texture;

    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        resources[resource_count++] = &app->renderer.render_target[i];
    }

    // samplers
    resources[resource_count++] = &app->renderer.sampler;
    resources[resource_count++] = &app->gui_ctx.resources.linear_sampler;

    if(!dm_renderer_upload_resources_to_heap(&app->context, resources, resource_count)) return false;

    return true;
}

void application_run(application *app)
{
    while(dm_is_running(&app->context))
    {
        /**************
         * BEGIN FRAME
         ***************/
        if(!dm_update_begin(&app->context)) break;

        dm_render_command_update_begin(&app->context);
            gui_new_frame(&app->context, &app->gui_ctx);

            instances_update(app->instances);

            if(!renderer_update(&app->renderer, &app->context, app->instances)) break;

            if(!gui_end_frame(&app->context, &app->gui_ctx)) break;
        dm_render_command_update_end(&app->context);

        /*********
         * RENDER
         **********/
        if(!dm_render_begin(&app->context)) break;

        renderer_render(&app->renderer, &app->context, &app->gui_ctx);
        //gui_render(&app->context, &app->gui_ctx, app->renderer.swapchain);

        if(!dm_render_end(&app->context))   break;

        /************
         * END FRAME
         *************/
        dm_update_end(&app->context);
    }
}

void application_shutdown(application *app)
{
    dm_shutdown(&app->context);

    dm_arena_detroy(&app->arena);
}
