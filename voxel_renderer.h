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
    vec2 position;
    vec2 uv;
} voxel_vertex;

typedef struct voxel_push_data_t
{
    u32 vb_index;
    u32 texture_index;
    u32 sampler_index;
} voxel_push_data;

typedef struct voxel_renderer_t
{
    voxel *voxels;
    u32   voxel_count;

    voxel_push_data push_data;
    u64 push_address;

    u32 *texture_data;
    u16 texture_width, texture_height;

    /*****************
     * RENDER HANDLES
     ******************/
    dm_handle vb_cpu, vb_gpu; 
    dm_handle ib_cpu, ib_gpu; 
    dm_handle pd_cpu, pd_gpu;

    dm_handle pipeline;
    dm_handle texture;
    dm_handle sampler;
} voxel_renderer;

bool voxel_renderer_init(voxel_renderer *renderer, dm_context *context, dm_arena *arena);
void voxel_renderer_update(voxel_renderer *renderer, dm_context *context);
void voxel_renderer_render(voxel_renderer *renderer, dm_context *context, dm_handle swapchain);

#endif // __VOXEL_RENDERER_H__
