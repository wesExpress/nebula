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

        b3ShapeDef shape_def = b3DefaultShapeDef();
        shape_def.density = 1.f;
        shape_def.baseMaterial.friction = 0.1f;

        b3BoxHull box = b3MakeBoxHull(scale[0] * 0.5, scale[1] * 0.5f, scale[2] * 0.5f);

        b3CreateHullShape(instances->bodies[i], &shape_def, &box.base);

        vec3 velocity = { random_float(), random_float(), random_float() };
        glm_vec3_scale(velocity, 2.f, velocity);
        glm_vec3_subs(velocity, 1.f, velocity);
        b3Body_SetLinearVelocity(instances->bodies[i], *(b3Vec3*)&velocity);
    }

    return true;
}

void instances_update(instance_data *instances)
{
    // apply gravity
    vec3 p_i, p_j;
    float m_i, m_j;

    const float g = 0.5f;

    for(u32 i=0; i<MAX_INSTANCES; i++)
    {
        b3WorldTransform transform_i = b3Body_GetTransform(instances->bodies[i]);
        m_i = b3Body_GetMass(instances->bodies[i]);

        for(u32 j=i+1; j<MAX_INSTANCES; j++)
        {
            b3WorldTransform transform_j = b3Body_GetTransform(instances->bodies[j]);
            m_j = b3Body_GetMass(instances->bodies[j]);
            
            // calculate force
            float dir_x = transform_j.p.x - transform_i.p.x; 
            float dir_y = transform_j.p.y - transform_i.p.y; 
            float dir_z = transform_j.p.z - transform_i.p.z; 

            float dis2 = dir_x * dir_x;
            dis2 += dir_y * dir_y;
            dis2 += dir_z * dir_z;

            float grav = g * m_i * m_j / dis2;

            dis2 = sqrt(dis2);
            dis2 = 1.f / dis2;

            dir_x *= dis2;
            dir_y *= dis2;
            dir_z *= dis2;

            b3Vec3 f_i = {  dir_x * grav,  dir_y * grav,  dir_z * grav };
            b3Vec3 f_j = { -dir_x * grav, -dir_y * grav, -dir_z * grav };

            b3Body_ApplyForceToCenter(instances->bodies[i], f_i, true);
            b3Body_ApplyForceToCenter(instances->bodies[j], f_j, true);
        }
    }

    //
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
