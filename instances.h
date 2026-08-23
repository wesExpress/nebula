#ifndef __INSTANCES_H__
#define __INSTANCES_H__

#include "DarkMatter/dm.h"
#include "cglm/cglm.h"

#include "box3d/box3d.h"

#define MAX_INSTANCES 1000

typedef struct instance_data_t
{
    b3WorldId world;

    b3BodyId bodies[MAX_INSTANCES];

    vec3 scales[MAX_INSTANCES];

    mat4 obj[MAX_INSTANCES][2];

} instance_data;

bool instances_init(instance_data *instances);
void instances_update(instance_data *instances);
void instances_shutdown(instance_data *instances);

#endif // __INSTANCES_H__
