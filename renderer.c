#include "renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

#include "vertex_data.h"

#include "random_gen.h"

bool renderer_init(render_data *renderer, dm_context *context)
{
    // swapchain
    dm_render_attachment_desc color_desc = {
        .load_op=DM_RENDER_ATTACHMENT_LOAD_OP_CLEAR,
        .store_op=DM_RENDER_ATTACHMENT_STORE_OP_STORE
    };
    dm_render_attachment_desc depth_desc = {
        .load_op=DM_RENDER_ATTACHMENT_LOAD_OP_LOAD,
        .store_op=DM_RENDER_ATTACHMENT_STORE_OP_DONT_CARE
    };
    dm_render_target_desc swapchain_desc = {
        .color_attachment=color_desc,
        .depth_attachment=depth_desc,
        .swapchain=true,
        .depth=true
    };
    if(!dm_renderer_create_render_target(context, swapchain_desc, &renderer->swapchain)) return false;
    
    // quad pipeline
    dm_raster_shader quad_vertex = {
        .path="../../assets/shaders/quad_vertex",
        .entry="v_main"
    };
    dm_raster_shader quad_fragment = {
        .path="../../assets/shaders/quad_fragment",
        .entry="f_main"
    };

    dm_raster_pipe_desc quad_pipe_desc = {
        .shaders[DM_RASTER_SHADER_STAGE_VERTEX]=quad_vertex,
        .shaders[DM_RASTER_SHADER_STAGE_FRAGMENT]=quad_fragment,

        .blend=true,
        .color_blend_op=DM_BLEND_OP_ADD,
        .color_src_factor=DM_BLEND_FACTOR_SRC_ALPHA,
        .color_dst_factor=DM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .alpha_blend_op=DM_BLEND_OP_ADD,
        .alpha_src_factor=DM_BLEND_FACTOR_ONE,
        .alpha_dst_factor=DM_BLEND_FACTOR_ZERO
    };

    if(!dm_renderer_create_raster_pipeline(context, quad_pipe_desc, &renderer->quad_pipeline)) return false;

    // render target
    dm_render_target_desc rt_desc = {
        .color_attachment.width=context->window.width,
        .color_attachment.load_op=DM_RENDER_ATTACHMENT_LOAD_OP_CLEAR,
        .color_attachment.store_op=DM_RENDER_ATTACHMENT_STORE_OP_STORE,
        .color_attachment.height=context->window.height,
        .depth_attachment.load_op=DM_RENDER_ATTACHMENT_LOAD_OP_CLEAR,
        .depth_attachment.store_op=DM_RENDER_ATTACHMENT_STORE_OP_DONT_CARE,
        .depth=true,
        .swapchain=false,
    };

    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        if(!dm_renderer_create_render_target(context, rt_desc, &renderer->render_target[i])) return false;
    }

    // object pipeline
    dm_raster_shader vertex_shader = {
        .path="../../assets/shaders/vertex",
        .entry="v_main"
    };
    dm_raster_shader fragment_shader = {
        .path="../../assets/shaders/fragment",
        .entry="f_main"
    };

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

    if(!dm_renderer_create_raster_pipeline(context, pipe_desc, &renderer->raster_pipeline)) return false;

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

    mat4 view, proj, view_proj;

    glm_look(renderer->cam_pos, renderer->cam_forward, renderer->cam_up, view);
    glm_perspective(renderer->fov, renderer->aspect, renderer->znear, renderer->zfar, proj);
    glm_mat4_mul(proj, view, view_proj);

    dm_buffer_desc cb_desc = {
        .type=DM_BUFFER_TYPE_STORAGE,
        .size=sizeof(mat4),
        .data=view_proj
    };

    dm_buffer_desc instb_desc = {
        .type=DM_BUFFER_TYPE_STORAGE,
        .size=sizeof(mat4) * MAX_INSTANCES * 2,
    };
    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        if(!dm_renderer_create_buffer(context, cb_desc, &renderer->cb[i])) return false;
        if(!dm_renderer_create_buffer(context, instb_desc, &renderer->instb[i])) return false;
    }

    // quad stuff
    u32 quad_indices[] = {
        0,1,2,
        3,0,2,
    };

    dm_buffer_desc quad_ib_desc = {
        .type=DM_BUFFER_TYPE_INDEX,
        .size=sizeof(quad_indices),
        .data=quad_indices
    };

    if(!dm_renderer_create_buffer(context, quad_ib_desc, &renderer->quad_ib)) return false;

    // submit resources
    dm_resource *resources[100] = { &renderer->vb, &renderer->ib, &renderer->quad_ib, &renderer->texture, &renderer->sampler };
    u32 resource_count = 5;
    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        resources[resource_count++] = &renderer->cb[i];
        resources[resource_count++] = &renderer->instb[i];
        resources[resource_count++] = &renderer->render_target[i];
    }

    if(!dm_renderer_upload_resources_to_heap(context, resources, resource_count)) return false;

    return true;
}

bool renderer_update(render_data *renderer, dm_context *context, instance_data *instances)
{
    const u8 current_frame = context->renderer.current_frame;

    if(dm_window_resized(context))
    {
        for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
        {
            if(!dm_render_command_update_texture(context, renderer->render_target[i], NULL, 0, context->window.width, context->window.height)) return false;
        }
    }

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

    mat4 view, proj, view_proj;

    renderer->aspect = (float)context->window.width / (float)context->window.height;

    glm_look(renderer->cam_pos, renderer->cam_forward, renderer->cam_up, view);
    glm_perspective(renderer->fov, renderer->aspect, renderer->znear, renderer->zfar, proj);
    glm_mat4_mul(proj, view, view_proj);

    dm_render_command_update_buffer(context, renderer->cb[current_frame], view_proj, sizeof(view_proj));

    dm_render_command_update_buffer(context, renderer->instb[current_frame], instances->obj, sizeof(mat4) * MAX_INSTANCES * 2);

    return true;
}

void renderer_render(render_data *renderer, dm_context *context)
{
    const u8 current_frame = context->renderer.current_frame;

    // render to texture
    dm_resource resources[] ={
        renderer->vb,
        renderer->instb[current_frame],
        renderer->cb[current_frame],
        renderer->texture,
        renderer->sampler,
    };

    dm_render_command_begin_rendering(context, renderer->render_target[current_frame], 0,0,0,1, 1.f);

        dm_render_command_bind_pipeline(context, renderer->raster_pipeline);
        dm_render_command_bind_index_buffer(context, renderer->ib, 0);
        dm_render_command_push_resources(context, resources, 5);
        dm_render_command_draw(context, 36, MAX_INSTANCES);
    
    dm_render_command_end_rendering(context, renderer->render_target[current_frame]);

    // draw to screen
    dm_resource quad_resources[] = {
        renderer->render_target[current_frame],
        renderer->sampler
    };

    dm_render_command_begin_rendering(context, renderer->swapchain, 1,0,1,1, 1);

        dm_render_command_bind_pipeline(context, renderer->quad_pipeline);
        dm_render_command_bind_index_buffer(context, renderer->quad_ib, 0);
        dm_render_command_push_resources(context, quad_resources, 2);
        dm_render_command_draw(context, 6, 1);

    dm_render_command_end_rendering(context, renderer->swapchain);
}
