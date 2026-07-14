#include "DarkMatter/dm.h"

#include "instances.h"
#include "random_gen.h"

void instances_init(instances *insts)
{
    const float world_size = 40.f;
    const float half_world = world_size * 0.5f;
    for(u32 i=0; i<MAX_INSTANCES; i++)
    {
        vec3 position = {
            random_float() * world_size - half_world,
            random_float() * world_size - half_world,
            random_float() * world_size - half_world
        };

        vec3 scale = { random_float(), random_float(), random_float() };
        vec3 axis = { random_float(), random_float(), random_float() };

        glm_vec3_dup(position, insts->positions[i]);
        glm_vec3_dup(scale, insts->scales[i]);
        glm_quatv(insts->orientations[i], random_float() * 3.14f * 2.f, axis);
    }
}

void instances_update(instances *insts)
{
    for(u32 i=0; i<MAX_INSTANCES; i++)
    {
        versor delta;
        glm_quatv(delta, 0.05f, (vec3){ 1,1,0 });
        glm_quat_mul(delta, insts->orientations[i], insts->orientations[i]);
        glm_quat_normalize(insts->orientations[i]);

        glm_mat4_identity(insts->obj[i][0]);

        glm_translate(insts->obj[i][0], insts->positions[i]);
        glm_quat_rotate(insts->obj[i][0], insts->orientations[i], insts->obj[i][0]);
        glm_scale(insts->obj[i][0], insts->scales[i]);

        glm_mat4_inv(insts->obj[i][0], insts->obj[i][1]);
        glm_mat4_transpose(insts->obj[i][1]);
    }
}
