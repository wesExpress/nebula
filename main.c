#include "application.h"

int main(void)
{
    application app = { 0 };

    if(application_init(&app, 1080, 720, "nebula"))
    {
        application_run(&app);
    }

    application_shutdown(&app);

    return 0;
}
