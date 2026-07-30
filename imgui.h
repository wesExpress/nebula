#ifndef __IMGUI_H__
#define __IMGUI_H__

#include "DarkMatter/dm.h"

#include "microui/microui.h"

#define IMGUI_MAX_VERTICES 1000
#define IMGUI_MAX_INDICES  3000

typedef u32 imgui_index;

typedef struct imgui_vertex_t
{
    float pos[2];
    float uv[2];
    float color[4];
} imgui_vertex;

typedef struct imgui_frame_data_t
{
    imgui_vertex vertices[IMGUI_MAX_VERTICES];
    imgui_index  indices[IMGUI_MAX_INDICES];

    u32 vertex_count, index_count;
} imgui_frame_data;

typedef struct imgui_context_t
{
    mu_Context *mu_ctx;

    imgui_frame_data frame_data[DM_FRAMES_IN_FLIGHT];

    // handles
    dm_pipeline pipeline;
    dm_resource vb[DM_FRAMES_IN_FLIGHT], ib[DM_FRAMES_IN_FLIGHT], font_texture;
    dm_resource scene[DM_FRAMES_IN_FLIGHT];
    dm_resource sampler;
} imgui_context;

bool imgui_init(dm_context *context, imgui_context *imgui_ctx);
void imgui_update(dm_context *context, imgui_context *imgui_ctx);
void imgui_render(dm_context *context, imgui_context *imgui_ctx, dm_resource render_target);
void imgui_shutdown(imgui_context *context);

#endif
