#include "voxel_renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

#include "vertex_data.h"

#include "random_gen.h"

bool voxel_renderer_init(voxel_renderer *renderer, dm_context *context, dm_arena *arena)
{
    dm_raster_shader vertex_shader = {
        .path="../../assets/shaders/vertex",
        .entry="v_main"
    };
    dm_raster_shader fragment_shader = {
        .path="../../assets/shaders/fragment",
        .entry="f_main"
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
    dm_buffer_desc vb_desc = {
        .type=DM_BUFFER_TYPE_VERTEX,
        .size=sizeof(cube_vertices),
        .data=cube_vertices
    };
    dm_buffer_desc ib_desc = {
        .type=DM_BUFFER_TYPE_INDEX,
        .size=sizeof(cube_indices),
        .data=cube_indices
    };
    if(!dm_renderer_create_buffer(context, vb_desc, &renderer->vb)) return false;
    if(!dm_renderer_create_buffer(context, ib_desc, &renderer->ib)) return false;

    // camera
    vec3 cam_pos     = { 0,0,10};
    vec3 cam_forward = { 0,0,-1 };
    vec3 cam_up      = { 0,1,0 };

    renderer->aspect = (float)context->window.width / (float)context->window.height;
    renderer->fov = glm_rad(70.f);
    renderer->znear = 0.1f;
    renderer->zfar = 100.f;

    glm_vec3_dup(cam_pos, renderer->cam_pos);
    glm_vec3_dup(cam_forward, renderer->cam_forward);
    glm_vec3_dup(cam_up, renderer->cam_up);

    mat4 view, proj;

    glm_look(renderer->cam_pos, renderer->cam_forward, renderer->cam_up, view);
    glm_perspective(renderer->fov, renderer->aspect, renderer->znear, renderer->zfar, proj);
    glm_mat4_mul(proj, view, renderer->scene_data.view_proj);

    dm_buffer_desc cb_desc = {
        .type=DM_BUFFER_TYPE_STORAGE,
        .size=sizeof(renderer->scene_data),
        .data=&renderer->scene_data
    };
    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        if(!dm_renderer_create_buffer(context, cb_desc, &renderer->cb[i])) return false;
    }

    const float world_size = 40.f;
    const float half_world = world_size * 0.5f;
    for(u32 i=0; i<MAX_INSTANCES; i++)
    {
        vec3 position = {
            random_float() * world_size - half_world,
            random_float() * world_size - half_world,
            random_float() * world_size - half_world
        };

        vec3 scale = { random_float(), random_float(), random_float() };
        vec3 axis = { random_float(), random_float(), random_float() };

        glm_vec3_dup(position, renderer->instances[i].position);
        glm_vec3_dup(scale, renderer->instances[i].scale);
        glm_quatv(renderer->instances[i].orientation, random_float() * 3.14f * 2.f, axis);
    }

    mat4 models[MAX_INSTANCES][2] = { 0 };

    dm_buffer_desc instb_desc = {
        .type=DM_BUFFER_TYPE_STORAGE,
        .size=sizeof(models),
        .data=models
    };
    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        if(!dm_renderer_create_buffer(context, instb_desc, &renderer->instb[i])) return false;
    }

    // push indices 
    for(u32 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        voxel_push_data push = {
            .vb=renderer->vb.gpu_index,
            .instb=renderer->instb[i].gpu_index,
            .scene=renderer->cb[i].gpu_index,
            .texture=renderer->texture.gpu_index,
            .sampler=renderer->sampler.gpu_index
        };

        dm_buffer_desc pd_desc = {
            .type=DM_BUFFER_TYPE_STORAGE,
            .size=sizeof(voxel_push_data),
            .data=&push
        };

        if(!dm_renderer_create_buffer(context, pd_desc, &renderer->pd[i])) return false;
    }

    // submit resources
    dm_resource *resources[100] = { &renderer->vb, &renderer->ib, &renderer->texture };
    u32 resource_count = 3;
    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        resources[resource_count++] = &renderer->cb[i];
        resources[resource_count++] = &renderer->instb[i];
        resources[resource_count++] = &renderer->pd[i];
    }

    dm_resource *samplers[] = { &renderer->sampler };

    if(!dm_renderer_upload_resources_to_heap(context, resources, resource_count)) return false;
    if(!dm_renderer_upload_samplers_to_heap(context, samplers, 1)) return false;

    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        renderer->push_address[i] = dm_renderer_get_buffer_address(context, renderer->pd[i]);
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

    renderer->aspect = (float)context->window.width / (float)context->window.height;

    glm_look(renderer->cam_pos, renderer->cam_forward, renderer->cam_up, view);
    glm_perspective(renderer->fov, renderer->aspect, renderer->znear, renderer->zfar, proj);
    glm_mat4_mul(proj, view, renderer->scene_data.view_proj);

    dm_render_command_update_buffer(context, renderer->cb[current_frame], &renderer->scene_data, sizeof(renderer->scene_data));

    mat4 models[MAX_INSTANCES][2] = { 0 };

    for(u32 i=0; i<MAX_INSTANCES; i++)
    {
        voxel_instance instance = renderer->instances[i];

        versor delta;
        glm_quatv(delta, 0.05f, (vec3){ 1,1,0 });
        glm_quat_mul(delta, instance.orientation, instance.orientation);
        glm_quat_normalize(instance.orientation);

        glm_mat4_identity(models[i][0]);

        glm_translate(models[i][0], instance.position);
        glm_quat_rotate(models[i][0], instance.orientation, models[i][0]);
        glm_scale(models[i][0], instance.scale);

        glm_mat4_inv(models[i][0], models[i][1]);
        glm_mat4_transpose(models[i][1]);

        renderer->instances[i] = instance;
    }

    dm_render_command_update_buffer(context, renderer->instb[current_frame], models, sizeof(models));

    return true;
}

void voxel_renderer_render(voxel_renderer *renderer, dm_context *context, dm_resource swapchain)
{
    const u8 current_frame = context->renderer.current_frame;

    // render
    dm_resource resources[] ={
        renderer->vb,
        renderer->instb[current_frame],
        renderer->cb[current_frame],
        renderer->texture,
        renderer->sampler,
    };
    dm_render_command_begin_rendering(context, swapchain, 0.f,0.f,0.f,1.f, 1.f);

        dm_render_command_bind_pipeline(context, renderer->pipeline);
        dm_render_command_bind_index_buffer(context, renderer->ib, 0);
        dm_render_command_push_resources(context, resources, 5);
        dm_render_command_draw(context, 36, MAX_INSTANCES);
    
    dm_render_command_end_rendering(context, swapchain);
}
