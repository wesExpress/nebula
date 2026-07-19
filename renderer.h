#ifndef __VOXEL_RENDERER_H__
#define __VOXEL_RENDERER_H__

#include "DarkMatter/dm.h"

#include "instances.h" 
#include "cglm/cglm.h"

typedef struct render_data_t
{
    vec3 cam_pos;
    vec3 cam_forward;
    vec3 cam_up;
    float fov;
    float aspect;
    float znear, zfar;

    double frame_time;
    u32 frame_count;

    /*****************
     * RENDER HANDLES
     ******************/
    dm_resource swapchain;

    dm_resource vb, ib, cb[DM_FRAMES_IN_FLIGHT];
    dm_resource instb[DM_FRAMES_IN_FLIGHT];
    dm_resource texture, sampler;

    dm_pipeline raster_pipeline, quad_pipeline;
    dm_resource render_target[DM_FRAMES_IN_FLIGHT];
    dm_resource quad_ib;

    dm_pipeline imgui_pipeline;
    dm_resource imgui_vb[DM_FRAMES_IN_FLIGHT], imgui_ib[DM_FRAMES_IN_FLIGHT];
    dm_resource imgui_constants;

    dm_pipeline compute_pipeline;

    dm_synchronization synchronization;
} render_data;

bool renderer_init(render_data *renderer, dm_context *context);
bool renderer_update(render_data *renderer, dm_context *context, instance_data* instances);
void renderer_render(render_data *renderer, dm_context *context);

#endif // __VOXEL_RENDERER_H__
