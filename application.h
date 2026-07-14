#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "DarkMatter/dm.h"
#include "renderer.h"
#include "instances.h"

typedef struct application_t
{
    dm_context  context;
    render_data renderer;
    instance_data instances;

    dm_arena arena;
} application;

bool application_init(application *app, u16 width, u16 height, const char *title);
void application_run(application *app);
void application_shutdown(application *app);

#endif
