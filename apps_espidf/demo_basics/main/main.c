// This is the demo driver that runs all of the basic dtcore demos.
// The demo logic is in separate files in the dtcore_demos/src folder.

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <dtcore/dtkvp.h>
#include <dtcore/dtlog.h>
#include <dtcore_demos/dtcore_demos.h>

#define TAG "dtcore_demo_basics"

void
app_main(void)
{
    dtcore_demos_demo_t demos[10] = { 0 };

    int ndemos = 0;
    DTCORE_DEMOS_ADD(dtcore_demo_hello);
    DTCORE_DEMOS_ADD(dtcore_demo_error);
    DTCORE_DEMOS_ADD(dtcore_demo_random);
    DTCORE_DEMOS_ADD(dtcore_demo_buffer);

    for (int i = 0; i < ndemos; i++)
    {
        printf("--------------------------------------------------------\n");
        dtlog_info(TAG, "running demo %d/%d %s...", i + 1, ndemos, demos[i].name);
        demos[i].func();
    }
}