#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <dtcore/dtlog.h>

extern void
dtcore_demo_hello(void);

void
app_main(void)
{
    // run the demo hello function
    dtcore_demo_hello();

    printf("END OF app_main\n\n");

    // Wait indefinitely to prevent the program from exiting.
    vTaskDelay(portMAX_DELAY);
}