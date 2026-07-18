#include <metal_stdlib>

using namespace metal;

struct argument_buffer
{
    texture2d<float, access::read_write> texture;
};

kernel void c_main(constant argument_buffer &arg[[buffer(0)]], uint2 gid [[thread_position_in_grid]])
{
    arg.texture.write(float4(1,1,0,1), gid);
}
