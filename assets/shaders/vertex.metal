#include <metal_stdlib>

using namespace metal;

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

struct vertex_out
{
    float4 position [[position]];
    float4 color;
    float3 world_pos;
    float3 normal;
    float2 uv;
};

struct scene_data
{
    float4x4 view_proj;
};

struct vertex_push_data
{
    device vertex_in *vertices;
    device instance *instances;
    device scene_data *constants;
};

struct argument_buffer
{
    device vertex_push_data *vpd;
};

vertex vertex_out v_main(constant argument_buffer& addresses [[buffer(0)]], uint vid [[vertex_id]], uint instid [[instance_id]])
{
    vertex_out v_out;

    vertex_in v   = addresses.vpd->vertices[vid];
    instance inst = addresses.vpd->instances[instid];

    float4x4 model     = inst.model;
    float4x4 inv_model = inst.inv_model;
    float4x4 view_proj = addresses.vpd->constants->view_proj;

    v_out.world_pos = (model * float4(v.position, 1)).xyz;
    v_out.position  = view_proj * model * float4(v.position, 1);
    v_out.normal    = (inv_model * float4(v.position, 0)).xyz;
    v_out.uv        = float2(v.u, v.v);
    v_out.color     = v.color;

    return v_out;
}
