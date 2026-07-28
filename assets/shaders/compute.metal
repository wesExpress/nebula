#include <metal_stdlib>

using namespace metal;

struct buffer_data
{
    float time;
};

struct argument_buffer
{
    device buffer_data *data;
    texture2d<float, access::read_write> texture;
};

kernel void c_main(constant argument_buffer &arg[[buffer(0)]], uint2 gid [[thread_position_in_grid]])
{
    if(gid.x >= arg.texture.get_width() || gid.y >= arg.texture.get_height()) return;

    float3 rgb = arg.texture.read(gid).rgb;
    float alpha = cos(arg.data->time) + 0.5f;

    arg.texture.write(float4(rgb,alpha), gid);
}
