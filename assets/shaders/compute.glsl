#version 460
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_descriptor_heap : require
#extension GL_EXT_nonuniform_qualifier : require

layout (descriptor_heap) uniform writeonly image2D texture_heap[];

layout (descriptor_heap) buffer compute_frame_t
{
    double time;
} compute_frame_data;

layout (push_constant) uniform push_data_t
{
    uint data_index;
    uint texture_index;
} push_data;

layout (local_size_x=16, local_size_y=16, local_size_z=1) in;

void main()
{
    ivec2 index = ivec2(gl_GlobalInvocationID.xy);
    imageStore(texture_heap[push_data.texture_index], index, vec4(1,1,0,sin(compute_frame_data[data_index].time)));
}
