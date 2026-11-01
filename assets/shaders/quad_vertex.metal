#include <metal_stdlib>

using namespace metal;

struct vertex_in
{
    packed_float2 position;
    packed_float2 uv;
    float4        color;
};

struct vertex_out
{
    float4 position [[position]];
    float2 uv;
    float4 color;
};

struct argument_buffer
{
    device vertex_in   *vertices;
    texture2d<float>    texture;
    sampler             s;
};

[[vertex]] 
vertex_out v_main(
    constant argument_buffer &arg[[buffer(0)]], 
    uint v_id [[vertex_id]])
{
    vertex_out v_out;

    vertex_in v = vertices[v_id];

    v_out.position = float4(v.position, 0, 1);
    v_out.uv       = v.uv;
    v_out.color    = v.color;

    return v_out;
}
