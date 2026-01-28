#include <dtcore/dtpicosdk_helper.h>

extern void
dtcore_demo_hello(void);

int
main(void)
{
    dtpicosdk_helper_init();
    dtpicosdk_helper_prompt_to_start();

    // run the demo hello function
    dtcore_demo_hello();

    dtpicosdk_helper_linger();

    return 0;
}