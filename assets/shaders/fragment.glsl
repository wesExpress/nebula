#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_descriptor_heap : require
#extension GL_EXT_nonuniform_qualifier : require

layout (location=0) in vec2 vertex_uv;
layout (location=1) in vec4 vertex_color;
layout (location=2) in vec3 vertex_normal;
layout (location=3) in vec3 vertex_position;

layout (location=0) out vec4 fragment_color;

layout(descriptor_heap) uniform texture2D texture_heap[];
layout(descriptor_heap) uniform sampler   sampler_heap[];

layout(push_constant) uniform push_data_t 
{
    uint vb_index;
    uint instb_index;
    uint scene_index;
    uint texture_index;
    uint sampler_index;
} push_data;

void main()
{
    fragment_color = texture(sampler2D(texture_heap[push_data.texture_index], sampler_heap[push_data.sampler_index]), vertex_uv);
    fragment_color *= vertex_color;

    vec3 dir = normalize(vec3(0,0,0) - vertex_position);

    fragment_color.rgb *= max(dot(dir, normalize(vertex_normal)), 0);
}
