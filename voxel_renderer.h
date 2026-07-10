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

typedef struct vertex_push_data_t
{
    u32 vb_index;
    u32 instb_index;
    u32 scene_index;
} vertex_push_data;

typedef struct fragment_push_data_t
{
    u32 texture_index;
    u32 sampler_index;
} fragment_push_data;

typedef struct voxel_push_data_t
{
    u64 vertex_data_address;
    u64 fragment_data_address;
} voxel_push_data;

typedef struct voxel_renderer_t
{
    voxel_push_data push_data;
    vertex_push_data vertex_data;
    fragment_push_data fragment_data;

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
    dm_handle vb, ib, vpd[DM_FRAMES_IN_FLIGHT], fpd, pd[DM_FRAMES_IN_FLIGHT], cb[DM_FRAMES_IN_FLIGHT];
    dm_handle instb[DM_FRAMES_IN_FLIGHT];

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
