#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_descriptor_heap : require
#extension GL_EXT_nonuniform_qualifier : require

layout (location=0) out vec2 vertex_uv;

layout (buffer_reference) readonly buffer constants
{
    uint vb_index;
    uint texture_index;
    uint sampler_index;
};

struct vertex
{
    vec2 position;
    vec2 uv;
};

layout (descriptor_heap) readonly buffer vertex_buffer_t
{
    vertex vertices[];
} vertex_buffer_heap[];

layout (push_constant) uniform push_data_t
{
    constants c;
} push_data;

void main()
{
    constants c = constants(push_data.c);

    gl_Position = vec4(vertex_buffer_heap[c.vb_index].vertices[gl_VertexIndex].position, 0, 1);
    vertex_uv   = vertex_buffer_heap[c.vb_index].vertices[gl_VertexIndex].uv;
}
