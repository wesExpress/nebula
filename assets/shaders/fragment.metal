#include <metal_stdlib>

using namespace metal;

struct vertex_out
{
    float4 position [[position]];
    float4 color;
    float3 world_pos;
    float3 normal;
    float2 uv;
};

struct fragment_out
{
    float4 color [[color(0)]];
};

struct fragment_push_data
{
    texture2d<float> texture;
    sampler          s;
};

struct argument_buffer
{
    device fragment_push_data *push_data;
};

fragment fragment_out f_main(vertex_out v [[stage_in]], device argument_buffer *address [[buffer(0)]])
{
    fragment_out f_out;

    f_out.color = v.color;

    return f_out;
}
