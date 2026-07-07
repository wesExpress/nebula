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
    uint camera_index;
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

layout (descriptor_heap) readonly buffer scene_data
{
    mat4 view_proj;
} scene_heap[];

layout (push_constant) uniform push_data_t
{
    constants c;
} push_data;

void main()
{
    constants c = constants(push_data.c);

    mat4 view_proj = scene_heap[c.camera_index].view_proj;
    mat4 model = mat4(1.f);

    vec4 position = vec4(vertex_buffer_heap[c.vb_index].vertices[gl_VertexIndex].position, 0, 1);
    position = model * position;
    position = view_proj * position;

    gl_Position = position;
    vertex_uv   = vertex_buffer_heap[c.vb_index].vertices[gl_VertexIndex].uv;
}
