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