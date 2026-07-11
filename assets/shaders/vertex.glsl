#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_descriptor_heap : require
#extension GL_EXT_nonuniform_qualifier : require

layout (location=0) out vec2 vertex_uv;
layout (location=1) out vec4 vertex_color;
layout (location=2) out vec3 vertex_normal;
layout (location=3) out vec3 vertex_position;

struct vertex
{
    vec3 position;
    float u;
    vec3 normal;
    float v;
    vec4 color;
};

struct instance
{
    mat4 model;
    mat4 inv_model;
};

layout (descriptor_heap) readonly buffer vertex_buffer_t
{
    vertex vertices[];
} vertex_buffer_heap[];

layout (descriptor_heap) readonly buffer instance_buffer_t
{
    instance instances[];
} instance_buffer_heap[];

layout (descriptor_heap) readonly buffer scene_data
{
    mat4 view_proj;
} scene_heap[];

layout (push_constant) uniform push_data_t 
{
    uint vb_index;
    uint instb_index;
    uint scene_index;
    uint texture_index;
    uint sampler_index;
} push_data;

void main()
{
    mat4 view_proj = scene_heap[push_data.scene_index].view_proj;
    mat4 model     = instance_buffer_heap[push_data.instb_index].instances[gl_InstanceIndex].model;
    mat4 inv_model = instance_buffer_heap[push_data.instb_index].instances[gl_InstanceIndex].inv_model;

    vec3 position = vertex_buffer_heap[push_data.vb_index].vertices[gl_VertexIndex].position;
    vec3 normal   = vertex_buffer_heap[push_data.vb_index].vertices[gl_VertexIndex].normal;
    vec4 color    = vertex_buffer_heap[push_data.vb_index].vertices[gl_VertexIndex].color;

    float u = vertex_buffer_heap[push_data.vb_index].vertices[gl_VertexIndex].u;
    float v = vertex_buffer_heap[push_data.vb_index].vertices[gl_VertexIndex].v;

    vec4 p = model * vec4(position, 1);

    vertex_position = p.xyz;
    gl_Position = view_proj * p; 

    vertex_uv = vec2(u,v);

    vertex_color = color;

    vertex_normal = (inv_model * vec4(normal, 0)).xyz;
}
