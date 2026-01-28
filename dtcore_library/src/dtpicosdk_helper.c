#include <stdio.h>

#include "hardware/watchdog.h"
#include "pico/bootrom.h" // for reset_usb_boot()
#include "pico/stdlib.h"

#include <dtcore/dtlog.h>

#include <dtcore/dtpicosdk_helper.h>

#define TAG "dtpicosdk_helper"

// ---------------------------------------------------------------------------------------------
void
dtpicosdk_helper_init(void)
{
    stdio_init_all();
    sleep_ms(300); // give host time to open CDC
}

// ---------------------------------------------------------------------------------------------
void
dtpicosdk_helper_prompt_to_start(void)
{
    int i = 0;
    while (true)
    {
        if (i % 10 == 0)
            printf("ready: press ' ' to start, 'r' to reboot, 'b' for BOOTSEL (%d)\n", i / 10);
        i++;

        if (dtpicosdk_helper_poll_keyboard() == ' ')
            break;

        sleep_ms(100);
    }
}

// ---------------------------------------------------------------------------------------------
void
dtpicosdk_helper_reboot(void)
{
    dtlog_info(TAG, "rebooting firmware...");
    sleep_ms(50);

    // Immediate software reset (restarts app)
    watchdog_reboot(0, 0, 0);
    while (true)
        tight_loop_contents();
}

// ---------------------------------------------------------------------------------------------
void
dtpicosdk_helper_bootsel(void)
{

    dtlog_info(TAG, "entering bootsel...");
    sleep_ms(50);

    // Re-enter the ROM bootloader's USB mass-storage mode
    // Params: interface=0 (USB), drive_strength=0 (default)
    reset_usb_boot(0, 0);

    while (true)
        tight_loop_contents();
}

// ---------------------------------------------------------------------------------------------
int
dtpicosdk_helper_poll_keyboard(void)
{
    int ch = getchar_timeout_us(0);
    if (ch == 'r')
    {
        dtpicosdk_helper_reboot();
    }
    else if (ch == 'b')
    {
        dtpicosdk_helper_bootsel();
    }
    return ch;
}

// ---------------------------------------------------------------------------------------------
void
dtpicosdk_helper_linger(void)
{
    dtlog_info(TAG, "finished: press 'r' to reboot, 'b' for BOOTSEL");
    while (true)
    {
        dtpicosdk_helper_poll_keyboard();
        sleep_ms(100);
    }
}
