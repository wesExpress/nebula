#include <metal_stdlib>

using namespace metal;

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

struct fragment_out
{
    float4 color [[color(0)]];
};

fragment fragment_out f_main(vertex_out v [[stage_in]], constant argument_buffer &arg[[buffer(0)]])
{
    fragment_out f_out;

    f_out.color = arg.texture.sample(arg.s, v.uv);

    return f_out;
}
