#include "DarkMatter/dm.h"

#include "instances.h"
#include "random_gen.h"

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__)
#include <immintrin.h>
#elif defined(__aarch64__)
#include <arm_neon.h>

void run_gravity_naive(instance_data *instances)
{
    float32x4_t mass_i, mass_j;
    float32x4_t px_i, py_i, pz_i;
    float32x4_t px_j, py_j, pz_j;
    float32x4_t local_x, local_y, local_z;
    float32x4_t r_x, r_y, r_z;
    float32x4_t dis2, grav;
    float32x4_t fx_j, fy_j, fz_j;

    static const float g = 0.5f;
    float32x4_t grav_const = vdupq_n_f32(g);
    float32x4_t ones = vdupq_n_f32(1.f);
    float32x4_t neg_ones = vdupq_n_f32(-1.f);

    float fx[MAX_INSTANCES] = { 0 };
    float fy[MAX_INSTANCES] = { 0 };
    float fz[MAX_INSTANCES] = { 0 };

    for(u32 i=0; i<MAX_INSTANCES; i++)
    {
        px_i = vdupq_n_f32(instances->px[i]);
        py_i = vdupq_n_f32(instances->py[i]);
        pz_i = vdupq_n_f32(instances->pz[i]);
        mass_i = vdupq_n_f32(instances->masses[i]);

        for(u32 j=i+1; j<MAX_INSTANCES-3; j+=4)
        {
            px_j = vld1q_f32(instances->px + j);
            py_j = vld1q_f32(instances->py + j);
            pz_j = vld1q_f32(instances->pz + j);
            mass_j = vld1q_f32(instances->masses + j);

            // direction vector
            r_x = vsubq_f32(px_j, px_i);
            r_y = vsubq_f32(py_j, py_i);
            r_z = vsubq_f32(pz_j, pz_i);

            // squared distance
            dis2 = vmulq_f32(r_x, r_x); 
            dis2 = vfmaq_f32(dis2, r_y, r_y);
            dis2 = vfmaq_f32(dis2, r_z, r_z);

            // G * m * m / d^2
            grav = vmulq_f32(grav_const, mass_i);
            grav = vmulq_f32(grav, mass_j);
            grav = vdivq_f32(grav, dis2);

            // normalize direction vector
            dis2 = vsqrtq_f32(dis2);
            dis2 = vdivq_f32(ones, dis2);
            r_x = vmulq_f32(r_x, dis2);
            r_y = vmulq_f32(r_y, dis2);
            r_z = vmulq_f32(r_z, dis2);

            // local force
            local_x = vmulq_f32(grav, r_x);
            local_y = vmulq_f32(grav, r_y);
            local_z = vmulq_f32(grav, r_z);

            // add forces
            float f_x[4], f_y[4], f_z[4];

            // i entities get all local forces
            fx[i] += vaddvq_f32(local_x);
            fy[i] += vaddvq_f32(local_y);
            fz[i] += vaddvq_f32(local_z);

            // j entities get negative of local
            fx_j = vld1q_f32(fx+j);
            fy_j = vld1q_f32(fy+j);
            fz_j = vld1q_f32(fz+j);

            fx_j = vsubq_f32(fx_j, local_x);
            fy_j = vsubq_f32(fy_j, local_y);
            fz_j = vsubq_f32(fz_j, local_z);

            vst1q_f32(fx+j, fx_j);
            vst1q_f32(fy+j, fy_j);
            vst1q_f32(fz+j, fz_j);
        }

        b3Vec3 force = { fx[i], fy[i], fz[i] };

        b3Body_ApplyForceToCenter(instances->bodies[i], force, true);
    }
}
#endif

bool instances_init(instance_data *instances)
{
    b3WorldDef world_def = b3DefaultWorldDef();
    world_def.gravity = (b3Vec3){0,0,0};
    world_def.workerCount = 4;

    instances->world = b3CreateWorld(&world_def);
    
    //
    const float world_size = 50.f;
    const float half_world = world_size * 0.5f;

    vec3 w = { 0, 0, .1f};

    for(u32 i=0; i<MAX_INSTANCES; i++)
    {
        float m = random_float();
        vec3 scale = { m,m,m };
        glm_vec3_dup(scale, instances->scales[i]);

        vec3 axis = { 0,1,0 };
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
        shape_def.density = m;
        shape_def.baseMaterial.friction = random_float();

        b3BoxHull box = b3MakeBoxHull(scale[0] * 0.5, scale[1] * 0.5f, scale[2] * 0.5f);

        b3CreateHullShape(instances->bodies[i], &shape_def, &box.base);

        vec3 velocity = { body_def.position.x, body_def.position.y, 0};
        glm_vec3_cross(w, velocity, velocity);
        b3Body_SetLinearVelocity(instances->bodies[i], *(b3Vec3*)&velocity);

        vec3 angular_velocity = { random_float(), random_float(), random_float() };
        glm_vec3_scale(angular_velocity, 5.f, angular_velocity);
        glm_vec3_subs(angular_velocity, 2.5f, angular_velocity);
        b3Body_SetAngularVelocity(instances->bodies[i], *(b3Vec3*)&angular_velocity);

        instances->px[i] = b3Body_GetTransform(instances->bodies[i]).p.x;
        instances->py[i] = b3Body_GetTransform(instances->bodies[i]).p.y;
        instances->pz[i] = b3Body_GetTransform(instances->bodies[i]).p.z;
        instances->masses[i] = b3Body_GetMass(instances->bodies[i]);
    }

    return true;
}

void instances_update(instance_data *instances)
{
    // apply gravity
    double start = dm_window_get_time();
    run_gravity_naive(instances);
    double grav_elapsed = dm_window_get_time() - start;

    //
    static const float time_step = 1.f / 60.f;
    int sub_step_count = 4;

    ImGuiIO *io = ImGui_GetIO();

    start = dm_window_get_time();
    b3World_Step(instances->world, io->DeltaTime, sub_step_count);
    double phys_elapsed = dm_window_get_time() - start;

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

        instances->px[i] = transform.p.x;
        instances->py[i] = transform.p.y;
        instances->pz[i] = transform.p.z;
        instances->masses[i] = b3Body_GetMass(instances->bodies[i]);
    }

    ImGui_Begin("Debug", NULL, 0);              

    ImGui_Text("Object count: %u", MAX_INSTANCES);
    ImGui_Text("Delta time: %lf ms", io->DeltaTime * 1000.f);
    ImGui_Text("Gravity: %lf ms", grav_elapsed * 1000.f);
    ImGui_Text("Box3D: %lf ms", phys_elapsed * 1000.f);

    ImGui_End();
}

void instances_shutdown(instance_data *instances)
{
    b3DestroyWorld(instances->world);
}
