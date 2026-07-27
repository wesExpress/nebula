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
    if(!imgui_init(&app->context, &app->imgui_context)) return false;

    /************
     * INSTANCES 
     *************/
    app->instances = dm_arena_alloc(&app->arena, sizeof(instance_data));
    instances_init(app->instances);

    // submit resources
    u32 resource_count = 0;

    dm_resource *resources[100] = { 0 };

    resources[resource_count++] = &app->renderer.vb;
    resources[resource_count++] = &app->renderer.ib;
    resources[resource_count++] = &app->renderer.quad_ib;
    resources[resource_count++] = &app->renderer.texture;
    resources[resource_count++] = &app->imgui_context.font_texture;
    resources[resource_count++] = &app->renderer.sampler;
    resources[resource_count++] = &app->imgui_context.sampler;

    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        resources[resource_count++] = &app->renderer.cb[i];
        resources[resource_count++] = &app->renderer.instb[i];
        resources[resource_count++] = &app->renderer.render_target[i];
        resources[resource_count++] = &app->imgui_context.vb[i];
        resources[resource_count++] = &app->imgui_context.ib[i];
        resources[resource_count++] = &app->imgui_context.scene[i];
    }

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

        imgui_update(&app->context, &app->imgui_context);
        instances_update(app->instances);

        if(!renderer_update(&app->renderer, &app->context, app->instances)) break;
        dm_render_command_update_end(&app->context);

        /*********
         * RENDER
         **********/
        if(!dm_render_begin(&app->context)) break;

        renderer_render(&app->renderer, &app->context, &app->imgui_context);
        imgui_render(&app->context, &app->imgui_context, app->renderer.swapchain);

        if(!dm_render_end(&app->context))   break;

        /************
         * END FRAME
         *************/
        dm_update_end(&app->context);
    }
}

void application_shutdown(application *app)
{
    imgui_shutdown(&app->imgui_context);
    dm_shutdown(&app->context);

    dm_arena_detroy(&app->arena);
}
