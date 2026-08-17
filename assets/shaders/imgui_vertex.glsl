#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_descriptor_heap : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require

layout (location=0) out vec2 vertex_uv;
layout (location=1) out vec4 vertex_color;

struct vertex
{
    vec2  position;
    vec2  uv;
    uint color;
};

layout (descriptor_heap, scalar) readonly buffer vertex_buffer_t
{
    vertex vertices[];
} vertex_buffer_heap[];

layout (descriptor_heap) readonly buffer scene_data
{
    mat4 ortho;
} scene_heap[];

layout (push_constant) uniform push_data_t
{
    uint vb_index;
    uint scene_index;
    uint texture_index;
    uint sampler_index;
} push_data;

void main()
{
    mat4 ortho = scene_heap[push_data.scene_index].ortho;

    vec4 position = vec4(vertex_buffer_heap[push_data.vb_index].vertices[gl_VertexIndex].position, 0, 1);
    gl_Position   = ortho * position;

    vertex_uv = vertex_buffer_heap[push_data.vb_index].vertices[gl_VertexIndex].uv;

    uint color = vertex_buffer_heap[push_data.vb_index].vertices[gl_VertexIndex].color;
    float r = (color >> 0)  & 0xFF;
    float g = (color >> 8)  & 0xFF;
    float b = (color >> 16) & 0xFF;
    float a = (color >> 24) & 0xFF;

    vertex_color = vec4(r,g,b,a) / vec4(255.f);
}
