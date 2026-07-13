#include <metal_stdlib>

using namespace metal;

struct vertex_in
{
    packed_float3 position;
    float  u;
    packed_float3 normal;
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
    texture2d<float> texture;
    sampler          s;
};

struct argument_buffer
{
    device vertex_in *vertices;
    device instance *instances;
    device scene_data *constants;
    texture2d<float> texture;
    sampler          s;
};

struct vertex_out
{
    float4 position [[position]];
    float4 color;
    float3 world_pos;
    float3 normal;
    float2 uv;
};

vertex vertex_out v_main(constant argument_buffer &arg[[buffer(0)]], uint v_id [[vertex_id]], uint inst_id [[instance_id]])
{
    vertex_out v_out;

    vertex_in v   = arg.vertices[v_id];
    instance inst = arg.instances[inst_id];

    float4x4 model     = inst.model;
    float4x4 inv_model = inst.inv_model;
    float4x4 view_proj = arg.constants->view_proj;

    v_out.world_pos = (model * float4(v.position, 1)).xyz;
    v_out.position  = view_proj * model * float4(v.position, 1);
    v_out.normal    = (inv_model * float4(v.normal, 0)).xyz;
    v_out.uv        = float2(v.u, v.v);
    v_out.color     = v.color;

    return v_out;
}
