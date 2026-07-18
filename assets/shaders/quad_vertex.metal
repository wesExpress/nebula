#include <metal_stdlib>

using namespace metal;

struct vertex_in
{
    packed_float2 position;
    packed_float2 uv;
};

struct vertex_out
{
    float4 position [[position]];
    float2 uv;
};

struct argument_buffer
{
    texture2d<float> texture;
    sampler          s;
};

static constant vertex_in vertices[] = {
    { { -1,-1 }, { 0,0 }, },
    { { -1, 1 }, { 0,1 }, },
    { {  1, 1 }, { 1,1 }, },
    { {  1,-1 }, { 1,0 }, },
};

[[vertex]] 
vertex_out v_main(
    constant argument_buffer &arg[[buffer(0)]], 
    uint v_id [[vertex_id]])
{
    vertex_out v_out;

    vertex_in v = vertices[v_id];

    v_out.position = float4(v.position, 0, 1);
    v_out.uv = v.uv;

    return v_out;
}
