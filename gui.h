#ifndef __GUI_H__
#define __GUI_H__

#include "DarkMatter/dm.h"

#define GUI_MAX_VERTICES 64000
#define GUI_MAX_INDICES  UINT16_MAX 

typedef struct gui_resources_t
{
    dm_pipeline pipeline;

    dm_resource vb[DM_FRAMES_IN_FLIGHT], ib[DM_FRAMES_IN_FLIGHT], scene[DM_FRAMES_IN_FLIGHT];
    dm_resource linear_sampler, nearest_sampler;

    dm_resource texture;
} gui_resources;

typedef struct gui_context_t
{
    gui_resources resources;
} gui_context;

bool gui_init(dm_context *context, gui_context *gui_ctx);
void gui_new_frame(dm_context *context, gui_context *gui_ctx);
bool gui_end_frame(dm_context *context, gui_context *gui_ctx);
void gui_render(dm_context *context, gui_context *gui_context);

#endif
