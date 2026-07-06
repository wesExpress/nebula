#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "DarkMatter/dm.h"
#include "voxel_renderer.h"

typedef struct application_t
{
    dm_arena arena;
    dm_context *context;
    size_t context_offset;

    voxel_renderer *renderer;
    size_t renderer_offset;

    /************
     * RENDERING 
     *************/
    dm_handle swapchain;
} application;

bool application_init(application *app, size_t size, u16 width, u16 height, const char *title);
void application_run(application *app);
void application_shutdown(application *app);

#endif
