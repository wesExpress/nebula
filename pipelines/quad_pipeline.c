#include "quad_pipeline.h"

#include "stb_image/stb_image.h"

typedef struct quad_inst_t
{
    mat4 model;
} quad_inst;

#define QUAD_MAX_INSTANCES 1000

bool quad_pipeline_init(dm_context *context, quad_pipeline *pipeline)
{
    // data
    const quad_vertex quad[] = {
        { { -0.5f,-0.5f }, { 0,0 }, { 1,1,1,1 } },
        { {  0.5f,-0.5f }, { 1,0 }, { 1,1,1,1 } },
        { {  0.5f, 0.5f }, { 1,1 }, { 1,1,1,1 } },
        { { -0.5f, 0.5f }, { 0,1 }, { 1,1,1,1 } },
    };

    const u32 quad_indices[] = {
        0,1,2,
        2,3,0,
    };

    // buffers
    const dm_buffer_desc vb_desc = {
        .type   = DM_BUFFER_TYPE_VERTEX,
        .stride = sizeof(quad_vertex),
        .size   = sizeof(pipeline->vertices),
        .data   = pipeline->vertices
    };

    const dm_buffer_desc ib_desc = {
        .type   = DM_BUFFER_TYPE_INDEX,
        .stride = sizeof(u32),
        .size   = sizeof(pipeline->indices),
        .data   = pipeline->indices
    };

    for(u8 i=0; i<DM_FRAMES_IN_FLIGHT; i++)
    {
        if(!dm_renderer_create_buffer(context, vb_desc, &pipeline->vb[i]))      return false;
        if(!dm_renderer_create_buffer(context, ib_desc, &pipeline->ib[i]))      return false;
    }
    
    // pipeline
    const dm_raster_shader quad_vertex = {
        .path="../../assets/shaders/quad_vertex",
        .entry="v_main"
    };

    const dm_raster_shader quad_fragment = { 
        .path="../../assets/shaders/quad_fragment",
        .entry="f_main"
    };

    const dm_raster_pipe_desc pipe_desc = {
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

    if(!dm_renderer_create_raster_pipeline(context, pipe_desc, &pipeline->pipe)) return false;

    // default texture
    u32 pixel = 0xFFFFFFFF;

    dm_texture2d_desc texture_desc = {
        .type   = DM_TEXTURE2D_TYPE_SAMPLED,
        .format = DM_TEXTURE2D_FORMAT_R8G8B8A8_UNORM,
        .height=1, .width=1,
        .data=&pixel
    };

    if(!dm_renderer_create_texture(context, texture_desc, &pipeline->texture)) return false;

    dm_sampler_desc sampler_desc = {
        .min=DM_SAMPLER_FILTER_LINEAR,
        .mag=DM_SAMPLER_FILTER_LINEAR,
        .mip=DM_SAMPLER_FILTER_LINEAR
    };

    if(!dm_renderer_create_sampler(context, sampler_desc, &pipeline->sampler)) return false;

    return true;
}

void quad_pipeline_submit_quad(dm_context *context, quad_pipeline *pipe, quad q)
{
    quad_vertex v0 = {
        .pos={q.x,q.y},
        .uv={0,0},
        .color={ q.color[0],q.color[1],q.color[2],q.color[3] }
    };
    quad_vertex v1 = {
        .pos={q.x + q.w,q.y},
        .uv={1,0},
        .color={ q.color[0],q.color[1],q.color[2],q.color[3] }
    };
    quad_vertex v2 = {
        .pos={q.x + q.w,q.y + q.h},
        .uv={1,1},
        .color={ q.color[0],q.color[1],q.color[2],q.color[3] }
    };
    quad_vertex v3 = {
        .pos={q.x,q.y + q.h},
        .uv={0,1},
        .color={ q.color[0],q.color[1],q.color[2],q.color[3] }
    };

    pipe->vertices[pipe->vertex_count+0] = v0;
    pipe->vertices[pipe->vertex_count+1] = v1;
    pipe->vertices[pipe->vertex_count+2] = v2;
    pipe->vertices[pipe->vertex_count+3] = v3;

    pipe->indices[pipe->index_count+0] = pipe->vertex_count+0;
    pipe->indices[pipe->index_count+1] = pipe->vertex_count+1;
    pipe->indices[pipe->index_count+2] = pipe->vertex_count+2;

    pipe->indices[pipe->index_count+3] = pipe->vertex_count+2;
    pipe->indices[pipe->index_count+4] = pipe->vertex_count+3;
    pipe->indices[pipe->index_count+5] = pipe->vertex_count+0;

    pipe->vertex_count += 4;
    pipe->index_count  += 6;
}

bool quad_pipeline_update_buffers(dm_context *context, quad_pipeline *pipe)
{
    const u8 frame = context->renderer.current_frame;

    dm_render_command_update_buffer(context, pipe->vb[frame], pipe->vertices, sizeof(pipe->vertices), 0);
    dm_render_command_update_buffer(context, pipe->ib[frame], pipe->indices,  sizeof(pipe->indices), 0);

    return true;
}

void quad_pipeline_draw_quads(dm_context *context, quad_pipeline *pipeline)
{
    const u8 frame = context->renderer.current_frame;

#define QUAD_PIPE_RESOURCES 3
    dm_resource resources[QUAD_PIPE_RESOURCES] = { 
        pipeline->vb[frame],
        pipeline->texture,
        pipeline->sampler
    };

    dm_render_command_bind_pipeline(context, pipeline->pipe);
    dm_render_command_bind_index_buffer(context, pipeline->ib[frame], 0);
    dm_render_command_push_resources(context, resources, QUAD_PIPE_RESOURCES);
    dm_render_command_draw(context, pipeline->index_count, 0, pipeline->vertex_count, 0);

    //
    pipeline->vertex_count = 0;
    pipeline->index_count  = 0;
#undef QUAD_PIPE_RESOURCES
}
