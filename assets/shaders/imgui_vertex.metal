#include <metal_stdlib>

using namespace metal;

struct vertex_in
{
    packed_float2 position;
    packed_float2 uv;
    uint   color;
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

[[vertex]]
vertex_out v_main(
    constant argument_buffer &arg[[buffer(0)]],
    uint v_id [[vertex_id]])
{
    vertex_out v_out;

    vertex_in v_in = arg.vertices[v_id];

    v_out.position = arg.scene->ortho * float4(v_in.position, 0, 1);
    v_out.uv = v_in.uv;

    float r = (v_in.color >> 0) & 0xff;
    float g = (v_in.color >> 8) & 0xff;
    float b = (v_in.color >> 16) & 0xff;
    float a = (v_in.color >> 24) & 0xff;

    v_out.color = float4(r,g,b,a) / float4(255.f);

    return v_out;
}
