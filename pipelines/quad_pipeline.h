#ifndef __QUAD_PIPELINE_H__
#define __QUAD_PIPELINE_H__

#include "DarkMatter/dm.h"
#include "cglm/cglm.h"

#define MAX_QUADS 1000
typedef struct quad_t
{
    vec4 color;
    u16  x,y,w,h;
} quad;

typedef struct quad_vertex_t
{
    vec2 pos;
    vec2 uv;
    vec4 color;
} quad_vertex;

typedef struct quad_pipeline_t
{
    dm_pipeline pipe;

    dm_resource vb[DM_FRAMES_IN_FLIGHT], ib[DM_FRAMES_IN_FLIGHT];
    dm_resource texture, sampler;

    quad_vertex vertices[MAX_QUADS * 4];
    u32         indices[MAX_QUADS * 6];
    u32         vertex_count, index_count;
} quad_pipeline;

bool quad_pipeline_init(dm_context *context, quad_pipeline *pipe);
void quad_pipeline_submit_quad(dm_context *context, quad_pipeline *pipe, quad q); 
bool quad_pipeline_update_buffers(dm_context *context, quad_pipeline *pipe);
void quad_pipeline_draw_quads(dm_context *context, quad_pipeline *pipeline);

#endif // __QUAD_PIPELINE_H__
