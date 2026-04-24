/*
demo_error: Error propagation and inspection

This demo focuses on how you do recoverable error reporting in dtcore.

The output you'd expect to see when running this demo is:

    [INFO ] dtcore_demo_error: caught the expected DTERR_ARGUMENT_NULL code

The demo_basics app runs this demo automatically.

Related modules: dterr, dtwallclock, dtlog
*/

#include <dtcore/dtlog.h>
#include <dtcore/dtwallclock.h>

#define TAG "dtcore_demo_error"

// ------------------------------------------------------------------------
void
dtcore_demo_error(void)
{
    dterr_t* dterr = NULL;

    // Intentionally cause an error by passing a NULL buffer
    dterr = dtwallclock_format_microseconds_as_hhmmss_llluuu(1234567ULL, NULL, 0);

    if (dterr == NULL || dterr->error_code != DTERR_ARGUMENT_NULL)
    {
        dtlog_error(TAG, "expected DTERR_ARGUMENT_NULL error but did not get it");
    }
    else
    {
        dtlog_info(TAG, "caught the expected DTERR_ARGUMENT_NULL code");
        dterr->dispose(dterr);
        dterr = NULL;
    }

    if (dterr != NULL)
    {
        dtlog_dterr(TAG, dterr);
        dterr->dispose(dterr);
        dterr = NULL;
    }
}
