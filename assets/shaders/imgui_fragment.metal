
#include <metal_stdlib>

using namespace metal;

struct vertex_in
{
    packed_float2 position;
    packed_float2 uv;
    float4 color;
};

struct imgui_scene
{
    float4x4 ortho;
};

struct argument_buffer
{
    device vertex_in   *vertices;
    device imgui_scene *scene;
    texture2d<float>    font_texture;
    sampler             s;
};

struct vertex_out
{
    float4 position [[position]];
    float2 uv;
    float4 color;
};

struct fragment_out
{
    float4 color [[color(0)]];
};

[[fragment]]
fragment_out f_main(
    vertex_out v [[stage_in]],
    constant argument_buffer &arg[[buffer(0)]])
{
    fragment_out f_out;

    f_out.color = v.color * arg.font_texture.sample(arg.s, v.uv);

    return f_out;
}
