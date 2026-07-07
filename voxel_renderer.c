#include "voxel_renderer.h"

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
    renderer->texture_width = context->window.width;
    renderer->texture_height = context->window.height;
    const size_t texture_size = sizeof(u32) * renderer->texture_width * renderer->texture_height;
    size_t offset;
    renderer->texture_data = malloc(texture_size);
    renderer->texture_data_size = texture_size;
    for(u32 x=0; x<renderer->texture_width; x++)
    {
        for(u32 y=0; y<renderer->texture_height; y++)
        {
            u32 color = 0;
            color |= 0xFF00FF00;

            renderer->texture_data[y * renderer->texture_width + x] = color;
        }
    }

    dm_texture2d_desc texture_desc = {
        .width=400,
        .height=400,
        .size=texture_size,
        .type=DM_TEXTURE2D_TYPE_SAMPLED,
        .data=renderer->texture_data
    };
    if(!dm_renderer_create_texture(context, texture_desc, &renderer->texture)) return false;

    // buffers
    voxel_vertex vertices[] = {
        { { -1,-1 }, { 0,0 } },
        { { -1, 1 }, { 0,1 } },
        { {  1, 1 }, { 1,1 } },
        { {  1,-1 }, { 1,0 } },
    };

    dm_buffer_desc vb_desc = {    
        .type=DM_BUFFER_TYPE_VERTEX,
        .reside=DM_BUFFER_RESIDE_CPU,
        .size=sizeof(vertices),
        .data=vertices
    };

    if(!dm_renderer_create_buffer(context, vb_desc, &renderer->vb_cpu)) return false;

    vb_desc.reside = DM_BUFFER_RESIDE_GPU;
    vb_desc.data   = NULL;
    if(!dm_renderer_create_buffer(context, vb_desc, &renderer->vb_gpu)) return false;

    u32 indices[] = {
        0,1,2,
        3,0,2
    };

    dm_buffer_desc ib_desc = {
        .type=DM_BUFFER_TYPE_INDEX,
        .reside=DM_BUFFER_RESIDE_CPU,
        .size=sizeof(indices),
        .data=indices
    };

    if(!dm_renderer_create_buffer(context, ib_desc, &renderer->ib_cpu)) return false;

    ib_desc.reside = DM_BUFFER_RESIDE_GPU;
    ib_desc.data   = NULL;
    if(!dm_renderer_create_buffer(context, ib_desc, &renderer->ib_gpu)) return false;

    dm_buffer_desc pd_desc = {
        .type=DM_BUFFER_TYPE_STORAGE,
        .reside=DM_BUFFER_RESIDE_CPU,
        .size=sizeof(voxel_push_data),
        .data=&renderer->push_data
    };

    if(!dm_renderer_create_buffer(context, pd_desc, &renderer->pd_cpu)) return false;

    pd_desc.reside = DM_BUFFER_RESIDE_GPU;
    if(!dm_renderer_create_buffer(context, pd_desc, &renderer->pd_gpu)) return false;

    // sampler
    dm_sampler_desc sampler_desc = { 0 };
    if(!dm_renderer_create_sampler(context, sampler_desc, &renderer->sampler)) return false;

    // submit resources
    dm_handle *resources[] = { &renderer->vb_gpu, &renderer->ib_gpu, &renderer->pd_gpu, &renderer->texture };
    dm_handle *samplers[]  = { &renderer->sampler };

    if(!dm_renderer_upload_resources_to_heap(context, resources, 4)) return false;
    if(!dm_renderer_upload_samplers_to_heap(context, samplers, 1)) return false;

    LOG_DEBUG("VB heap index: %u", renderer->vb_gpu.heap_index);
    LOG_DEBUG("IB heap index: %u", renderer->ib_gpu.heap_index);
    LOG_DEBUG("PD heap index: %u", renderer->pd_gpu.heap_index);
    LOG_DEBUG("Texture heap index: %u", renderer->texture.heap_index);
    LOG_DEBUG("Sampler heap index: %u", renderer->sampler.heap_index);

    // copy over to gpu
    dm_render_command_copy_buffer(context, renderer->vb_cpu, renderer->vb_gpu);
    dm_render_command_copy_buffer(context, renderer->ib_cpu, renderer->ib_gpu);

    // push indices
    renderer->push_data.vb_index      = renderer->vb_gpu.heap_index;
    renderer->push_data.texture_index = renderer->texture.heap_index;
    renderer->push_data.sampler_index = renderer->sampler.heap_index;

    dm_render_command_update_buffer(context, renderer->pd_cpu, &renderer->push_data, sizeof(voxel_push_data));
    dm_render_command_copy_buffer(context, renderer->pd_cpu, renderer->pd_gpu);

    renderer->push_address = dm_renderer_get_buffer_address(context, renderer->pd_gpu);

    return true;
}

bool voxel_renderer_update(voxel_renderer *renderer, dm_context *context)
{
    if(dm_window_resized(context))
    {
        const u16 width  = context->window.width;
        const u16 height = context->window.height;
        const size_t data_size = sizeof(u32) * width * height;

        renderer->texture_data = realloc(renderer->texture_data, data_size);
        renderer->texture_data_size = data_size;

        renderer->texture_width  = width;
        renderer->texture_height = height;
    }

    const u16 width = renderer->texture_width;
    const u16 height = renderer->texture_height;

    for(u32 x=0; x<width; x++)
    {
        for(u32 y=0; y<height; y++)
        {
            u32 color = 0xFFFF00FF;

            renderer->texture_data[y * width + x] = color;
        }
    }

    if(!dm_render_command_update_texture(context, renderer->texture, renderer->texture_data, renderer->texture_data_size, width, height)) return false;

    return true;
}

void voxel_renderer_render(voxel_renderer *renderer, dm_context *context, dm_handle swapchain)
{
    // render
    dm_render_command_begin_rendering(context, swapchain, 0.1f, 0.2f, 0.2f, 1.f, 1.f);

        dm_render_command_bind_pipeline(context, renderer->pipeline);
        dm_render_command_bind_index_buffer(context, renderer->ib_gpu, 0);
        dm_render_command_push_data(context, &renderer->push_address, sizeof(renderer->push_address));
        dm_render_command_draw(context, 6, 1);
    
    dm_render_command_end_rendering(context, swapchain);
}
