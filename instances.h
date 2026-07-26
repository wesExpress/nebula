#ifndef __INSTANCES_H__
#define __INSTANCES_H__

#include "DarkMatter/dm.h"
#include "cglm/cglm.h"

#define MAX_INSTANCES 8000

typedef struct instance_data_t
{
    vec3   positions[MAX_INSTANCES];
    vec3   scales[MAX_INSTANCES];
    versor orientations[MAX_INSTANCES];

    mat4 obj[MAX_INSTANCES][2];
} instance_data;

bool instances_init(instance_data *instances);
void instances_update(instance_data *instances);

#endif // __INSTANCES_H__
