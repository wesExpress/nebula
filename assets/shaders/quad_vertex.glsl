#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_descriptor_heap : require
#extension GL_EXT_nonuniform_qualifier : require

layout (location=0) out vec2 vertex_uv;

struct vertex
{
    vec2 position;
    vec2 uv;
};

layout (push_constant) uniform push_data_t
{
    uint texture_index;
    uint sampler_index;
} push_data;

vertex vertices[] = {
    { { -1,-1 }, { 0,0 } },
    { { -1, 1 }, { 0,1 } },
    { {  1, 1 }, { 1,1 } },
    { {  1,-1 }, { 1,0 } },
};

void main()
{
    gl_Position = vec4(vertices[gl_VertexIndex].position, 0, 1.f);

    vertex_uv = vertices[gl_VertexIndex].uv;
}
