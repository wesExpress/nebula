#include "application.h"
#include "random_gen.h"

bool application_init(application *app, u16 width, u16 height, const char *title)
{
    random_init();

    const size_t instance_data_size = P_SIZE + S_SIZE + O_SIZE + M_SIZE;

    dm_arena_create(&app->arena, instance_data_size + DM_KILABYTE);

    /*************
     * DARKMATTER
     **************/
    if(!dm_init(&app->context, width, height, title, 0)) return false;

    /***********
     * RENDERER
     ************/
    if(!renderer_init(&app->renderer, &app->context)) return false;

    /************
     * INSTANCES 
     *************/
    instances_init(&app->instances, &app->arena);

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

        instances_update(&app->instances);

        if(!renderer_update(&app->renderer, &app->context, &app->instances)) break;

        /*********
         * RENDER
         **********/
        if(!dm_render_begin(&app->context)) break;

        renderer_render(&app->renderer, &app->context);

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
