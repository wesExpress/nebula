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

struct vertex_in
{
    float3 position;
    float  u;
    float3 normal;
    float  v;
    float4 color;
};

struct instance
{
    float4x4 model;
    float4x4 inv_model;
};

struct scene_data
{
    float4x4 view_proj;
};

struct resources
{
    device vertex_in *vertices;
    device instance *instances;
    device scene_data *constants;
    //texture2d<float> texture;
    //sampler          s;
};

struct argument_buffer
{
    //device resources *addresses;
    device vertex_in *vertices;
    device instance *instances;
    device scene_data *constants;
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
    f_out.color *= v.color;

    float3 dir = normalize(float3(0,0,0) - v.world_pos);

    f_out.color.rgb *= max(dot(dir, normalize(v.normal)), 0.f);

    return f_out;
}
