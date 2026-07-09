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
typedef struct voxel_instance_t
{
    vec3 position;
    vec3 scale;
    versor orientation;
} voxel_instance;

typedef struct voxel_scene_data_t
{
    mat4 view_proj;
} voxel_scene_data;

typedef struct voxel_push_data_t
{
    u32 vb_index;
    u32 instb_index;
    u32 texture_index;
    u32 sampler_index;
    u32 scene_index;
} voxel_push_data;

typedef struct voxel_buffer_t
{
    dm_handle cpu, gpu;
} voxel_buffer;

typedef struct voxel_renderer_t
{
    voxel_push_data push_data;
    u64 push_address;

    vec3 cam_pos;
    vec3 cam_forward;
    vec3 cam_up;
    float fov;
    float aspect;
    float znear, zfar;

    voxel_scene_data scene_data;

    voxel_instance instances[MAX_INSTANCES];

    double frame_time;
    u32 frame_count;

    /*****************
     * RENDER HANDLES
     ******************/
    voxel_buffer vb, ib, pd[DM_FRAMES_IN_FLIGHT], cb[DM_FRAMES_IN_FLIGHT];
    voxel_buffer instb[DM_FRAMES_IN_FLIGHT];

    dm_handle pipeline;
    dm_handle texture;
    dm_handle sampler;

    /*****************
     * DYNAMIC MEMORY
     ******************/
    voxel *voxels;
    u32   voxel_count;
} voxel_renderer;

bool voxel_renderer_init(voxel_renderer *renderer, dm_context *context, dm_arena *arena);
bool voxel_renderer_update(voxel_renderer *renderer, dm_context *context);
void voxel_renderer_render(voxel_renderer *renderer, dm_context *context, dm_handle swapchain);

#endif // __VOXEL_RENDERER_H__
