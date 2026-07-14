#include "DarkMatter/dm.h"
#include "application.h"
#include "renderer.h"

#ifndef NDEBUG
#define NEBDEBUG
#endif

int main(void)
{
    dm_arena arena = { 0 };
    dm_arena_create(&arena, sizeof(application) + DM_KILABYTE);

    application *app = dm_arena_alloc(&arena, sizeof(application), NULL);

    if(application_init(app, 1080, 720, "nebula"))
    {
        application_run(app);
    }

    application_shutdown(app);

    //
    dm_arena_detroy(&arena);

    return 0;
}
