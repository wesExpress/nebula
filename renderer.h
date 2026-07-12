#ifndef __VOXEL_RENDERER_H__
#define __VOXEL_RENDERER_H__

#include "DarkMatter/dm.h"

#include "cglm/cglm.h"

typedef struct voxel_t
{
    u32 data;
} voxel;

#define MAX_INSTANCES 8000

typedef struct scene_data_t
{
    mat4 view_proj;
} scene_data;

typedef struct renderer_t
{
    vec3 cam_pos;
    vec3 cam_forward;
    vec3 cam_up;
    float fov;
    float aspect;
    float znear, zfar;

    scene_data scene_data;

    vec3 positions[MAX_INSTANCES];
    vec3 scales[MAX_INSTANCES];
    versor orientations[MAX_INSTANCES];

    double frame_time;
    u32 frame_count;

    /*****************
     * RENDER HANDLES
     ******************/
    dm_resource vb, ib, cb[DM_FRAMES_IN_FLIGHT];
    dm_resource instb[DM_FRAMES_IN_FLIGHT];
    dm_resource texture, sampler;

    dm_pipeline raster_pipeline, compute_pipeline;

    /*****************
     * DYNAMIC MEMORY
     ******************/
    voxel *voxels;
    u32   voxel_count;
} renderer_t;

bool renderer_init(renderer_t *renderer, dm_context *context, dm_arena *arena);
bool renderer_update(renderer_t *renderer, dm_context *context);
void renderer_render(renderer_t *renderer, dm_context *context, dm_resource swapchain);

#endif // __VOXEL_RENDERER_H__
