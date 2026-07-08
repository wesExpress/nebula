#include "voxel_renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

bool create_buffer(dm_context *context, voxel_buffer *buffer, dm_buffer_type type, void *data, size_t size)
{
    dm_buffer_desc cpu_desc = {
        .type=DM_BUFFER_TYPE_STORAGE,
        .reside=DM_BUFFER_RESIDE_CPU,
        .size=size,
        .data=data
    };
    if(!dm_renderer_create_buffer(context, cpu_desc, &buffer->cpu)) return false;

    dm_buffer_desc gpu_desc = {
        .type=type,
        .reside=DM_BUFFER_RESIDE_GPU,
        .size=size
    };
    if(!dm_renderer_create_buffer(context, gpu_desc, &buffer->gpu)) return false;

    dm_render_command_copy_buffer(context, buffer->cpu, buffer->gpu);

    return true;
}

bool voxel_renderer_init(voxel_renderer *renderer, dm_context *context, dm_arena *arena)
{
    dm_raster_shader vertex_shader = {
        .path="../../assets/shaders/voxel_vertex.glsl",
        .entry="main"
    };
    dm_raster_shader fragment_shader = {
        .path="../../assets/shaders/voxel_fragment.glsl",
        .entry="main"
    };

    dm_raster_shader shaders[] = { vertex_shader, fragment_shader };

    dm_raster_pipe_desc pipe_desc = {
        .shaders[DM_RASTER_SHADER_STAGE_VERTEX]=vertex_shader,
        .shaders[DM_RASTER_SHADER_STAGE_FRAGMENT]=fragment_shader,

        .blend=true,
        .color_blend_op=DM_BLEND_OP_ADD,
        .color_src_factor=DM_BLEND_FACTOR_SRC_ALPHA,
        .color_dst_factor=DM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .alpha_blend_op=DM_BLEND_OP_ADD,
        .alpha_src_factor=DM_BLEND_FACTOR_ONE,
        .alpha_dst_factor=DM_BLEND_FACTOR_ZERO
    };

    if(!dm_renderer_create_raster_pipeline(context, pipe_desc, &renderer->pipeline)) return false;

    // texture
    int w,h,n;

    void *texture_data = stbi_load("../../assets/textures/grid.jpg", &w, &h, &n, 4);
    if(!texture_data) return false;

    dm_texture2d_desc texture_desc = {
        .width=w,
        .height=h,
        .size=sizeof(u32) * w * h,
        .type=DM_TEXTURE2D_TYPE_SAMPLED,
        .data=texture_data
    };
    if(!dm_renderer_create_texture(context, texture_desc, &renderer->texture)) return false;

    stbi_image_free(texture_data);

    // sampler
    dm_sampler_desc sampler_desc = { 0 };
    if(!dm_renderer_create_sampler(context, sampler_desc, &renderer->sampler)) return false;

    // buffers
    voxel_vertex vertices[] = {
        { { -1,-1,0,  0 }, { 1,0,0,  0 } },
        { { -1, 1,0,  0 }, { 0,1,0,  1 } },
        { {  1, 1,0,  1 }, { 0,0,1,  1 } },
        { {  1,-1,0,  1 }, { 0,1,1,  0 } },
    };

    u32 indices[] = {
        0,1,2,
        3,0,2
    };

    if(!create_buffer(context, &renderer->vb, DM_BUFFER_TYPE_VERTEX, vertices, sizeof(vertices))) return false;
    if(!create_buffer(context, &renderer->ib, DM_BUFFER_TYPE_INDEX,  indices,  sizeof(indices)))  return false;

    // camera
    vec3 cam_pos     = { 0,0,5};
    vec3 cam_forward = { 0,0,-1 };
    vec3 cam_up      = { 0,1,0 };

    renderer->aspect = (float)context->window.width / (float)context->window.height;
    renderer->fov = 70.f;
    renderer->znear = 0.01f;
    renderer->zfar = 100.f;

    glm_vec3_dup(cam_pos, renderer->cam_pos);
    glm_vec3_dup(cam_forward, renderer->cam_forward);
    glm_vec3_dup(cam_up, renderer->cam_up);

    mat4 view, proj;

    glm_look(renderer->cam_pos, renderer->cam_forward, renderer->cam_up, view);
    glm_perspective(renderer->fov, renderer->aspect, renderer->znear, renderer->zfar, proj);
    glm_mat4_mul(proj, view, renderer->scene_data.view_proj);

    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        if(!create_buffer(context, &renderer->cb[i], DM_BUFFER_TYPE_STORAGE, &renderer->scene_data, sizeof(renderer->scene_data))) return false;
    }

    // submit resources
    dm_handle *resources[10] = { &renderer->vb.gpu, &renderer->texture };
    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        resources[2+i] = &renderer->cb[i].gpu;
    }
    dm_handle *samplers[]  = { &renderer->sampler };

    if(!dm_renderer_upload_resources_to_heap(context, resources, 2+DM_FRAMES_IN_FLIGHT)) return false;
    if(!dm_renderer_upload_samplers_to_heap(context, samplers, 1)) return false;

    // push indices
    renderer->push_data.vb_index      = renderer->vb.gpu.heap_index;
    renderer->push_data.texture_index = renderer->texture.heap_index;
    renderer->push_data.sampler_index = renderer->sampler.heap_index; 

    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        renderer->push_data.camera_index  = renderer->cb[i].gpu.heap_index;

        if(!create_buffer(context, &renderer->pd[i], DM_BUFFER_TYPE_STORAGE, &renderer->push_data, sizeof(voxel_push_data))) return false;
    }

    return true;
}

bool voxel_renderer_update(voxel_renderer *renderer, dm_context *context)
{
    const u8 current_frame = context->renderer.current_frame;

    if(dm_is_key_pressed(context, 65))
    {
        renderer->cam_pos[0] -= 0.1f;
    }
    else if(dm_is_key_pressed(context, 68))
    {
        renderer->cam_pos[0] += 0.1f;
    }

    if(dm_is_key_pressed(context, 87))
    {
        renderer->cam_pos[2] -= 0.1f;
    }
    else if(dm_is_key_pressed(context, 83))
    {
        renderer->cam_pos[2] += 0.1f;
    }

    mat4 view, proj;

    glm_look(renderer->cam_pos, renderer->cam_forward, renderer->cam_up, view);
    glm_perspective(renderer->fov, renderer->aspect, renderer->znear, renderer->zfar, proj);
    glm_mat4_mul(proj, view, renderer->scene_data.view_proj);

    dm_render_command_update_buffer(context, renderer->cb[current_frame].cpu, &renderer->scene_data, sizeof(renderer->scene_data));
    dm_render_command_copy_buffer(context, renderer->cb[current_frame].cpu, renderer->cb[current_frame].gpu);

    return true;
}

void voxel_renderer_render(voxel_renderer *renderer, dm_context *context, dm_handle swapchain)
{
    const u8 current_frame = context->renderer.current_frame;

    // render
    renderer->push_address = dm_renderer_get_buffer_address(context, renderer->pd[current_frame].gpu);

    dm_render_command_begin_rendering(context, swapchain, 0.1f, 0.2f, 0.2f, 1.f, 1.f);

        dm_render_command_bind_pipeline(context, renderer->pipeline);
        dm_render_command_bind_index_buffer(context, renderer->ib.gpu, 0);
        dm_render_command_push_data(context, &renderer->push_address, sizeof(renderer->push_address));
        dm_render_command_draw(context, 6, 1);
    
    dm_render_command_end_rendering(context, swapchain);
}
