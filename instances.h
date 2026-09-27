#ifndef __INSTANCES_H__
#define __INSTANCES_H__

#include "DarkMatter/dm.h"
#define CGLM_ALL_UNALIGNED
#include "cglm/cglm.h"

#include "box3d/box3d.h"

#define MAX_INSTANCES (8 << 9)

typedef struct instance_data_t
{
    mat4 obj[MAX_INSTANCES][2];
    vec3 scales[MAX_INSTANCES];

    b3WorldId world;
    b3BodyId bodies[MAX_INSTANCES];

    float px[MAX_INSTANCES], py[MAX_INSTANCES], pz[MAX_INSTANCES];
    float masses[MAX_INSTANCES];
} instance_data;

bool instances_init(instance_data *instances);
void instances_update(instance_data *instances);
void instances_shutdown(instance_data *instances);

#endif // __INSTANCES_H__
