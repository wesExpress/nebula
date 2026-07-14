#include "DarkMatter/dm.h"

#include "instances.h"
#include "random_gen.h"

bool instances_init(instance_data *instances, dm_arena *arena)
{
    instances->positions = dm_arena_alloc(arena, P_SIZE);
    if(!instances->positions) return false;

    instances->scales = dm_arena_alloc(arena, S_SIZE);
    if(!instances->scales) return false;

    instances->orientations = dm_arena_alloc(arena, O_SIZE);
    if(!instances->orientations) return false;

    instances->obj = dm_arena_alloc(arena, M_SIZE);
    if(!instances->obj) return false;

    const float world_size = 150.f;
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

        glm_vec3_dup(position, instances->positions[i]);
        glm_vec3_dup(scale, instances->scales[i]);
        glm_quatv(instances->orientations[i], random_float() * 3.14f * 2.f, axis);
    }

    return true;
}

void instances_update(instance_data *instances)
{
    for(u32 i=0; i<MAX_INSTANCES; i++)
    {
        versor delta;
        glm_quatv(delta, 0.05f, (vec3){ 1,1,0 });
        glm_quat_mul(delta, instances->orientations[i], instances->orientations[i]);
        glm_quat_normalize(instances->orientations[i]);

        glm_mat4_identity(instances->obj[i][0]);

        glm_translate(instances->obj[i][0], instances->positions[i]);
        glm_quat_rotate(instances->obj[i][0], instances->orientations[i], instances->obj[i][0]);
        glm_scale(instances->obj[i][0], instances->scales[i]);

        glm_mat4_inv(instances->obj[i][0], instances->obj[i][1]);
        glm_mat4_transpose(instances->obj[i][1]);
    }
}

