#include <metal_stdlib>

using namespace metal;

struct vertex_out
{
    float4 position [[position]];
    float4 color;
    float4 world_pos;
    float4 normal;
    float2 uv;
};

struct argument_buffer
{
    device float *vertices;
    device float *instances;
    device float *constants;
    texture2d<float> texture;
    sampler          s;
};

struct fragment_out
{
    float4 color [[color(0)]];
};

[[fragment]] 
fragment_out f_main(
    vertex_out v_out [[stage_in]], 
    constant argument_buffer &arg[[buffer(0)]])
{
    fragment_out f_out;

    f_out.color = arg.texture.sample(arg.s, v_out.uv);
    f_out.color *= v_out.color;

    float3 dir = normalize(float3(0,0,0) - v_out.world_pos.xyz);

    f_out.color.rgb *= max(dot(dir, normalize(v_out.normal.xyz)), 0.f);

    return f_out;
}
