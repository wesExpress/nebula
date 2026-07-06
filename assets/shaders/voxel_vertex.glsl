#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_descriptor_heap : require

layout(location=0) vec2 vertex_uv;

layout(buffer_reference) readonly buffer constants
{
    u32 vb_index;
    u32 texture_index;
    u32 sampler_index;
};

layout(push_constant) uniform push_data_t
{
    constants c;
} push_data;

struct vertex
{
    vec2 pos;
    vec2 uv;
};

layout(descriptor_heap) readonly buffer vertex_buffer_t
{
    vertex vertices[];
} vertex_buffer_heap[];

void main()
{
}
