#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_descriptor_heap : require
#extension GL_EXT_nonuniform_qualifier : require

layout(location=0) in vec2 vertex_uv;
layout(location=0) out vec4 fragment_color;

layout(buffer_reference) readonly buffer constants
{
    uint vb_index;
    uint texture_index;
    uint sampler_index;
};

layout(push_constant) uniform push_data_t
{
    constants c;
} push_data;

layout(descriptor_heap) uniform texture2D texture_heap[];
layout(descriptor_heap) uniform sampler   sampler_heap[];

void main()
{
    constants c = constants(push_data.c);

    fragment_color = texture(sampler2D(texture_heap[c.texture_index], sampler_heap[c.sampler_index]), vertex_uv);

    //fragment_color = vec4(vertex_uv, 0, 1);
    fragment_color = vec4(1,1,1,1);
}
