#ifndef __IMGUI_H__
#define __IMGUI_H__

#include "DarkMatter/dm.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "Nuklear/nuklear.h"

typedef struct imgui_vertex_t
{
    float pos[2];
    float uv[2];
    float color[4];
} imgui_vertex;

typedef struct imgui_context_t
{
    struct nk_context nuklear_context;
    struct nk_font_atlas font_atlas;
    struct nk_buffer commands;
    struct nk_draw_null_texture null_texture;

    int max_vertex_buffer, int_max_index_buffer;

    imgui_vertex *vertices[DM_FRAMES_IN_FLIGHT];
    u16 *indices[DM_FRAMES_IN_FLIGHT];

    // handles
    dm_pipeline pipeline;
    dm_resource vb[DM_FRAMES_IN_FLIGHT], ib[DM_FRAMES_IN_FLIGHT], font_texture;
    dm_resource scene[DM_FRAMES_IN_FLIGHT];
    dm_resource sampler;
} imgui_context;

bool imgui_init(dm_context *context, imgui_context *imgui_ctx);
void imgui_update(dm_context *context, imgui_context *imgui_ctx);
void imgui_render(dm_context *context, imgui_context *imgui_ctx);
void imgui_shutdown(imgui_context *context);

#endif
