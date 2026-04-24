/*
demo_random: Facade-based component usage

This demo highlights dtcore's facade pattern in practice.

It also shows use of an expanding string via dtstr.

The output you'd expect to see when running this demo is similar to:

    [INFO ] dtcore_demo_random: generated random numbers: -2, -5, -1, 6, -4

Related modules: dtrandomizer, dtrandomizer_uniform, dtstr, dtlog
*/

#include <dtcore/dtcore_constants.h>

#include <dtcore/dtlog.h>
#include <dtcore/dtrandomizer.h>
#include <dtcore/dtrandomizer_uniform.h>
#include <dtcore/dtstr.h>

#define TAG "dtcore_demo_random"

// ------------------------------------------------------------------------
void
dtcore_demo_random(void)
{
    dtrandomizer_handle randomizer = NULL;
    dterr_t* dterr = NULL;
    char* expanding_string = NULL;

    {
        // create the randomizer
        dtrandomizer_uniform_t* o = NULL;
        DTERR_C(dtrandomizer_uniform_create(&o));

        // use the object as a handle for the facade
        randomizer = (dtrandomizer_handle)o;

        // configure the randomizer to give numbers in [-100, 100) with step at most 10
        dtrandomizer_uniform_config_t c = { 0 };
        c.edge = 100;
        c.step = 10;
        // use the same seed every time for demo repeatability
        c.seed = 12345;
        DTERR_C(dtrandomizer_uniform_config((dtrandomizer_uniform_t*)randomizer, &c));
    }

    // generate some random numbers and append them to a growing string
    for (int i = 0; i < 5; i++)
    {
        int32_t r = 0;
        DTERR_C(dtrandomizer_next(randomizer, &r));
        expanding_string = dtstr_concat_format(expanding_string, ", ", "%d", r);
    }

    dtlog_info(TAG, "generated random numbers: %s", expanding_string);

cleanup:
    // dispose of the randomizer
    dtrandomizer_dispose(randomizer);
    dtstr_dispose(expanding_string);

    if (dterr != NULL)
    {
        dtlog_dterr(TAG, dterr);
        dterr->dispose(dterr);
        dterr = NULL;
    }
}