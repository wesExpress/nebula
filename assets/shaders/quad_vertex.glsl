#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_descriptor_heap : require
#extension GL_EXT_nonuniform_qualifier : require

layout (location=0) out vec2 vertex_uv;
layout (location=1) out vec4 vertex_color;

struct vertex
{
    vec2 position;
    vec2 uv;
    vec4 color;
};

layout (descriptor_heap) readonly only vertex_buffer_t
{
    vertex vertices[];
} vertex_buffer_heap[];

layout (descriptor_heap) readonly only scene_data
{
    mat4 ortho;
};

layout (push_constant) uniform push_data_t
{
    uint vb_index;
    uint scene_data;
    uint texture_index;
    uint sampler_index;
} push_data;

void main()
{
    
}
