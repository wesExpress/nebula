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
    float4 world_pos;
    float4 normal;
    float2 uv;
};

[[vertex]] 
vertex_out v_main(
    constant argument_buffer &arg[[buffer(0)]], 
    uint v_id [[vertex_id]], 
    uint inst_id [[instance_id]])
{
    vertex_out v_out;

    vertex_in v_in = arg.vertices[v_id];
    instance inst  = arg.instances[inst_id];

    float4x4 model     = inst.model;
    float4x4 inv_model = inst.inv_model;
    float4x4 view_proj = arg.constants->view_proj;

    v_out.world_pos = model * float4(v_in.position, 1);
    v_out.position  = view_proj * model * float4(v_in.position, 1);
    v_out.normal    = inv_model * float4(v_in.normal, 0);
    v_out.uv        = float2(v_in.u, v_in.v);
    v_out.color     = v_in.color;

    return v_out;
}
