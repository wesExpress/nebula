#include "application.h"

#ifndef NDEBUG
#define NEBDEBUG
#endif

int main(void)
{
    application app = { 0 };

    if(application_init(&app, DM_MEGABYTE, 400, 400, "nebula"))
    {
        application_run(&app);
    }

    application_shutdown(&app);

    return 0;
}
