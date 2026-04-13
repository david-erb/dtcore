/*
 * dtpicosdk_helper -- RP2040 Pico SDK convenience helpers.
 *
 * Provides initialization, reboot, bootloader entry, and keyboard polling
 * wrappers for the Raspberry Pi Pico SDK.  Simplifies common startup and
 * interactive-prompt patterns used in Pico-targeted firmware.
 *
 * cdox v1.0.2
 */
#pragma once

extern void
dtpicosdk_helper_init();
void
dtpicosdk_helper_prompt_to_start(void);
extern void
dtpicosdk_helper_reboot(void);
extern void
dtpicosdk_helper_bootsel(void);
extern int
dtpicosdk_helper_poll_keyboard(void);
extern void
dtpicosdk_helper_linger(void);