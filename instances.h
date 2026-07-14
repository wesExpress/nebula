#ifndef __INSTANCES_H__
#define __INSTANCES_H__

#include "cglm/cglm.h"

#define MAX_INSTANCES 8000

typedef struct instances_t
{
    vec3 positions[MAX_INSTANCES];
    vec3 scales[MAX_INSTANCES];
    versor orientations[MAX_INSTANCES];

    mat4 obj[MAX_INSTANCES][2];
} instances;

void instances_init(instances *insts);
void instances_update(instances *insts);

#endif // __INSTANCES_H__
