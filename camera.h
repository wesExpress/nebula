#ifndef __CAMERA_H__
#define __CAMERA_H__

//#define CGLM_ALL_UNALIGNED
#include "cglm/cglm.h"

typedef struct perspective_camera_t
{
    mat4 view, projection, view_projection;

    vec3 position, forward, up, padding;

    float fov, zfar, znear, aspect_ratio;
} perspective_camera;

perspective_camera camera_init(vec3 position, vec3 forward, vec3 up, float fov, float aspect_ratio, float zfar, float znear);
void camera_update(perspective_camera *camera);

#endif // __CAMERA_H__
