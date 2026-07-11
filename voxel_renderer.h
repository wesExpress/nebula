#ifndef __VOXEL_RENDERER_H__
#define __VOXEL_RENDERER_H__

#include "DarkMatter/dm.h"

#include "cglm/cglm.h"

typedef struct voxel_t
{
    u32 data;
} voxel;

typedef struct voxel_vertex_t
{
    vec4 position_u;
    vec4 normal_v;
    vec4 color;
} voxel_vertex;

#define MAX_INSTANCES 8000

typedef struct voxel_scene_data_t
{
    mat4 view_proj;
} voxel_scene_data;

typedef struct voxel_renderer_t
{
    vec3 cam_pos;
    vec3 cam_forward;
    vec3 cam_up;
    float fov;
    float aspect;
    float znear, zfar;

    voxel_scene_data scene_data;

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

    dm_pipeline pipeline;

    /*****************
     * DYNAMIC MEMORY
     ******************/
    voxel *voxels;
    u32   voxel_count;
} voxel_renderer;

bool voxel_renderer_init(voxel_renderer *renderer, dm_context *context, dm_arena *arena);
bool voxel_renderer_update(voxel_renderer *renderer, dm_context *context);
void voxel_renderer_render(voxel_renderer *renderer, dm_context *context, dm_resource swapchain);

#endif // __VOXEL_RENDERER_H__
