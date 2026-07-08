#include "application.h"
#include "voxel_renderer.h"

#ifndef NDEBUG
#define NEBDEBUG
#endif

int main(void)
{
    const size_t arena_size = sizeof(dm_context) + sizeof(voxel_renderer) + DM_KILABYTE;

    application app = { 0 };

    if(application_init(&app, arena_size, 400, 400, "nebula"))
    {
        application_run(&app);
    }

    application_shutdown(&app);

    return 0;
}
