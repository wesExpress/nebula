#ifndef __VOXEL_RENDERER_H__
#define __VOXEL_RENDERER_H__

#include "DarkMatter/dm.h"

#include "cglm/cglm.h"

#define MAX_INSTANCES 8000

typedef struct renderer_t
{
    vec3 cam_pos;
    vec3 cam_forward;
    vec3 cam_up;
    float fov;
    float aspect;
    float znear, zfar;

    vec3 positions[MAX_INSTANCES];
    vec3 scales[MAX_INSTANCES];
    versor orientations[MAX_INSTANCES];

    double frame_time;
    u32 frame_count;

    /*****************
     * RENDER HANDLES
     ******************/
    dm_resource swapchain;

    dm_resource vb, ib, cb[DM_FRAMES_IN_FLIGHT];
    dm_resource instb[DM_FRAMES_IN_FLIGHT];
    dm_resource texture, sampler;

    dm_pipeline raster_pipeline, compute_pipeline, quad_pipeline;
    dm_resource render_target[DM_FRAMES_IN_FLIGHT];
    dm_resource quad_ib;
} renderer_t;

bool renderer_init(renderer_t *renderer, dm_context *context, dm_arena *arena);
bool renderer_update(renderer_t *renderer, dm_context *context);
void renderer_render(renderer_t *renderer, dm_context *context);

#endif // __VOXEL_RENDERER_H__
