#include "renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

#include "vertex_data.h"

#define GRID_X 16
#define GRID_Y 16
#define GRID_Z 1

bool renderer_init(render_data *renderer, dm_context *context)
{
    // render target
    dm_render_target_desc rt_desc = {
        .swapchain=false,
        .color_attachment.width=context->window.width,
        .color_attachment.height=context->window.height,
        .depth=true,
    };

    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        if(!dm_renderer_create_render_target(context, rt_desc, &renderer->render_target[i])) return false;
    }

    // swapchain
    dm_render_target_desc swapchain_desc = {
        .swapchain=true,
        .depth=true,
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
        .alpha_src_factor=DM_BLEND_FACTOR_SRC_ALPHA,
        .alpha_dst_factor=DM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,

        .winding=DM_WINDING_COUNTERCLOCKWISE,
        .culling=DM_CULL_BACK,
        .fill=DM_FILL_FULL,
        .primitive_type=DM_PRIMITIVE_TRIANGLE_LIST
    };

    if(!dm_renderer_create_raster_pipeline(context, quad_pipe_desc, &renderer->quad_pipeline)) return false;

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

        .depth=true,

        .blend=true,
        .color_blend_op=DM_BLEND_OP_ADD,
        .color_src_factor=DM_BLEND_FACTOR_SRC_ALPHA,
        .color_dst_factor=DM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .alpha_blend_op=DM_BLEND_OP_ADD,
        .alpha_src_factor=DM_BLEND_FACTOR_SRC_ALPHA,
        .alpha_dst_factor=DM_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,

        .winding=DM_WINDING_COUNTERCLOCKWISE,
        .culling=DM_CULL_BACK,
        .fill=DM_FILL_FULL,
        .primitive_type=DM_PRIMITIVE_TRIANGLE_LIST
    };

    if(!dm_renderer_create_raster_pipeline(context, pipe_desc, &renderer->raster_pipeline)) return false;

    // compute
    dm_compute_pipeline_desc compute_desc = {
        .shader.path="../../assets/shaders/compute",
        .shader.entry="c_main",
        .grp_x=GRID_X,.grp_y=GRID_Y,.grp_z=GRID_Z
    };
    if(!dm_renderer_create_compute_pipeline(context, compute_desc, &renderer->compute_pipeline)) return false;

    dm_buffer_desc compute_buffer = {
        .type=DM_BUFFER_TYPE_STORAGE,
        .size=sizeof(float),
        .stride=sizeof(float),
    };

    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        if(!dm_renderer_create_buffer(context, compute_buffer, &renderer->compute_frame_data[i])) return false;
    }

    // texture
    int w,h,n;

    void *texture_data = stbi_load("../../assets/textures/grid.jpg", &w, &h, &n, 4);
    if(!texture_data) return false;

    dm_texture2d_desc texture_desc = {
        .type=DM_TEXTURE2D_TYPE_SAMPLED,
        .format=DM_TEXTURE2D_FORMAT_R8G8B8A8_UNORM,
        .width=w,
        .height=h,
        .data=texture_data
    };
    if(!dm_renderer_create_texture(context, texture_desc, &renderer->texture)) return false;

    stbi_image_free(texture_data);

    // sampler
    dm_sampler_desc sampler_desc = { 
        .min=DM_SAMPLER_FILTER_NEAREST,
        .mag=DM_SAMPLER_FILTER_NEAREST,
        .mip=DM_SAMPLER_FILTER_NEAREST
    };
    if(!dm_renderer_create_sampler(context, sampler_desc, &renderer->sampler)) return false;

    // buffers
    dm_buffer_desc vb_desc = {
        .type=DM_BUFFER_TYPE_VERTEX,
        .size=sizeof(cube_vertices),
        .stride=sizeof(vertex),
        .data=cube_vertices
    };
    dm_buffer_desc ib_desc = {
        .type=DM_BUFFER_TYPE_INDEX,
        .size=sizeof(cube_indices),
        .stride=sizeof(cube_indices[0]),
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
        .stride=2 * sizeof(mat4)
    };
    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        if(!dm_renderer_create_buffer(context, cb_desc, &renderer->cb[i])) return false;
        if(!dm_renderer_create_buffer(context, instb_desc, &renderer->instb[i])) return false;
    }

    // quad stuff
    u32 quad_indices[] = {
        0,1,2,
        2,3,0,
    };

    dm_buffer_desc quad_ib_desc = {
        .type=DM_BUFFER_TYPE_INDEX,
        .size=sizeof(quad_indices),
        .stride=sizeof(quad_indices[0]),
        .data=quad_indices
    };

    if(!dm_renderer_create_buffer(context, quad_ib_desc, &renderer->quad_ib)) return false;

    // synchronization
    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        dm_synchronization_desc desc = { 0 };

        if(!dm_renderer_create_synchronization(context, desc, &renderer->synchronization[i])) return false;
    }

    return true;
}

bool renderer_update(render_data *renderer, dm_context *context, instance_data *instances)
{
    const u8 current_frame = context->renderer.current_frame;

    if(dm_window_resized(context))
    {
        const u16 width = context->window.width;
        const u16 height = context->window.height;

        for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
        {
            dm_resource target = renderer->render_target[i];
            
            if(!dm_render_command_resize_render_target(context, target, width, height)) return false;
        }
    }

    if(dm_key_is_pressed(context, DM_KEY_A))
    {
        renderer->cam_pos[0] -= 0.1f;
    }
    else if(dm_key_is_pressed(context, DM_KEY_D))
    {
        renderer->cam_pos[0] += 0.1f;
    }

    if(dm_key_is_pressed(context, DM_KEY_W))
    {
        renderer->cam_pos[2] -= 0.1f;
    }
    else if(dm_key_is_pressed(context, DM_KEY_S))
    {
        renderer->cam_pos[2] += 0.1f;
    }

    mat4 view, proj, view_proj;

    renderer->aspect = (float)context->window.width / (float)context->window.height;

    glm_look(renderer->cam_pos, renderer->cam_forward, renderer->cam_up, view);
    glm_perspective(renderer->fov, renderer->aspect, renderer->znear, renderer->zfar, proj);
    glm_mat4_mul(proj, view, view_proj);

    dm_render_command_update_buffer(context, renderer->cb[current_frame], view_proj, sizeof(view_proj), 0);

    const size_t obj_size = sizeof(mat4) * MAX_INSTANCES * 2;
    dm_render_command_update_buffer(context, renderer->instb[current_frame], instances->obj, obj_size, 0);

    renderer->frame_time += 3.1415926535f / 180.f;
    dm_render_command_update_buffer(context, renderer->compute_frame_data[current_frame], &renderer->frame_time, sizeof(float), 0);

    return true;
}

void renderer_render(render_data *renderer, dm_context *context, gui_context *gui_ctx)
{
    const u8 current_frame = context->renderer.current_frame;
    dm_resource render_target = renderer->render_target[current_frame];

    // render to texture
    dm_resource resources[] ={
        renderer->vb,
        renderer->instb[current_frame],
        renderer->cb[current_frame],
        renderer->texture,
        renderer->sampler,
    };

    const int width = context->window.width;
    const int height = context->window.height;

    dm_render_command_begin_rendering(context, render_target, 0,0,0,1, 1.f, DM_RENDER_LOAD_OP_CLEAR, DM_RENDER_STORE_OP_STORE, DM_RENDER_LOAD_OP_CLEAR, DM_RENDER_STORE_OP_STORE);
        dm_render_command_set_viewport(context, 0,0, width, height, 0, 1.f);
        dm_render_command_set_scissor(context, 0,0, width, height);
        dm_render_command_bind_pipeline(context, renderer->raster_pipeline);
        dm_render_command_bind_index_buffer(context, renderer->ib, 0);
        dm_render_command_push_resources(context, resources, 5);
        dm_render_command_draw(context, 36, 0, MAX_INSTANCES, 0);
    dm_render_command_end_rendering(context, render_target);

    dm_render_command_signal(context, renderer->synchronization[current_frame]);

    // compute time
    dm_resource compute_resources[] = {
        renderer->compute_frame_data[current_frame],
        render_target
    };

    const u16 dx = (context->window.width / 2 + GRID_X - 1) / GRID_X;
    const u16 dy = (context->window.height + GRID_Y - 1) / GRID_Y;
    const u16 dz = GRID_Z;

#if 0
    dm_compute_command_wait(context, renderer->synchronization[current_frame]);

    dm_compute_command_begin_recording(context);
        dm_compute_command_bind_pipeline(context, renderer->compute_pipeline);
        dm_compute_command_push_resources(context, compute_resources, 2);
        dm_compute_command_dispatch(context, dx, dy, dz);
    dm_compute_command_end_recording(context);

    dm_compute_command_signal(context, renderer->synchronization[current_frame]);
#endif

    // draw to screen
    dm_resource quad_resources[] = {
        render_target,
        renderer->sampler
    };

    dm_render_command_wait(context, renderer->synchronization[current_frame]);

    dm_render_command_begin_rendering(context, renderer->swapchain, 1,0,1,1, 1, DM_RENDER_LOAD_OP_CLEAR, DM_RENDER_STORE_OP_STORE, DM_RENDER_LOAD_OP_CLEAR, DM_RENDER_STORE_OP_DONT_CARE);
        dm_render_command_set_viewport(context, 0,0, width, height, 0, 1.f);
        dm_render_command_set_scissor(context, 0,0, width, height);
        dm_render_command_bind_pipeline(context, renderer->quad_pipeline);
        dm_render_command_bind_index_buffer(context, renderer->quad_ib, 0);
        dm_render_command_push_resources(context, quad_resources, 2);
        dm_render_command_draw(context, 6, 0, 1, 0);

        gui_render(context, gui_ctx);

    dm_render_command_end_rendering(context, renderer->swapchain);
}
