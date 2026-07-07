#include "camera.h"

perspective_camera camera_init(vec3 position, vec3 forward, vec3 up, float fov, float aspect_ratio, float zfar, float znear)
{
    perspective_camera camera = {  
        .fov=fov,
        .aspect_ratio=aspect_ratio,
        .zfar=zfar,
        .znear=znear
    };

    glm_vec3_dup(position, camera.position);
    glm_vec3_dup(forward, camera.forward);
    glm_vec3_dup(up, camera.up);

    glm_look(position, forward, up, camera.view);
    glm_perspective(fov, aspect_ratio, znear, zfar, camera.projection);
    glm_mat4_mul(camera.projection, camera.view, camera.view_projection);

    return camera;
}

void camera_update(perspective_camera *camera)
{
    glm_look(camera->position, camera->forward, camera->up, camera->view);
    glm_mat4_mul(camera->projection, camera->view, camera->view_projection);
}
