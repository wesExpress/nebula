#ifndef __INSTANCES_H__
#define __INSTANCES_H__

#include "DarkMatter/dm.h"
#include "cglm/cglm.h"

#define MAX_INSTANCES 1000000

#define P_SIZE (sizeof(vec3) * MAX_INSTANCES)
#define S_SIZE (sizeof(vec3) * MAX_INSTANCES)
#define O_SIZE (sizeof(versor) * MAX_INSTANCES)
#define M_SIZE (sizeof(mat4) * MAX_INSTANCES * 2)

typedef struct instance_data_t
{
    vec3 *positions;
    vec3 *scales;
    versor *orientations;

    mat4 (*obj)[2];
} instance_data;

bool instances_init(instance_data *instances, dm_arena *arena);
void instances_update(instance_data *instances);

#endif // __INSTANCES_H__
