#include "DarkMatter/dm.h"

#include "instances.h"
#include "random_gen.h"

bool instances_init(instance_data *instances)
{
    b3WorldDef world_def = b3DefaultWorldDef();
    world_def.gravity = (b3Vec3){0,0,0};

    instances->world = b3CreateWorld(&world_def);
    
    //
    const float world_size = 50.f;
    const float half_world = world_size * 0.5f;
    for(u32 i=0; i<MAX_INSTANCES; i++)
    {
        vec3 scale = { random_float(), random_float(), random_float() };
        glm_vec3_dup(scale, instances->scales[i]);

        vec3 axis = { random_float(), random_float(), random_float() };
        versor q;
        glm_quatv(q, random_float() * 3.14f * 2.f, axis);

        b3BodyDef body_def = b3DefaultBodyDef();
        body_def.type = b3_dynamicBody;
        body_def.position = (b3Vec3){
            random_float() * world_size - half_world,
            random_float() * world_size - half_world,
            random_float() * world_size - half_world
        };
        body_def.rotation = *(b3Quat*)&q;

        instances->bodies[i] = b3CreateBody(instances->world, &body_def);

        b3BoxHull box = b3MakeBoxHull(scale[0], scale[1], scale[2]);

        b3ShapeDef shape_def = b3DefaultShapeDef();
        shape_def.density = 1.f;
        shape_def.baseMaterial.friction = 0.1f;

        b3CreateHullShape(instances->bodies[i], &shape_def, &box.base);
    }

    return true;
}

void instances_update(instance_data *instances)
{
    static const float time_step = 1.f / 60.f;
    int sub_step_count = 4;

    b3World_Step(instances->world, time_step, sub_step_count);

    for(u32 i=0; i<MAX_INSTANCES; i++)
    {
        b3WorldTransform transform = b3Body_GetTransform(instances->bodies[i]);

        b3Vec3 position = transform.p;
        b3Quat orientation = transform.q;

        glm_mat4_identity(instances->obj[i][0]);

        glm_translate(instances->obj[i][0], *(vec3*)&position);
        glm_quat_rotate(instances->obj[i][0], *(versor*)&orientation, instances->obj[i][0]);
        glm_scale(instances->obj[i][0], instances->scales[i]);

        glm_mat4_inv(instances->obj[i][0], instances->obj[i][1]);
        glm_mat4_transpose(instances->obj[i][1]);
    }
}

void instances_shutdown(instance_data *instances)
{
    b3DestroyWorld(instances->world);
}
