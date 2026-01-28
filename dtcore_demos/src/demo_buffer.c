/*
demo_buffer: Buffer allocation and usage

This demo shows how to use dtbuffer to allocate space on the heap and refer to its intrinsic length.

You also get to see how a log line is emitted using dtlog.

The output you'd expect to see when running this demo is:

    [INFO ] dtcore_demo_buffer: buffered payload is: I'm in the dtcore buffer!  My total size is 128 bytes.

The demo_basics app runs this demo automatically.

Related modules: dtbuffer, dtlog
*/

#include <stdio.h>

#include <dtcore/dtbuffer.h>
#include <dtcore/dtlog.h>

#define TAG "dtcore_demo_buffer"

// ------------------------------------------------------------------------
void
dtcore_demo_buffer(void)
{
    dterr_t* dterr = NULL;
    dtbuffer_t* buffer = NULL;

    // allocate a buffer of 128 bytes
    DTERR_C(dtbuffer_create(&buffer, 128));

    // fill the buffer with some data
    snprintf(buffer->payload, buffer->length, "I'm in the dtcore buffer!  My total size is %" PRId32 " bytes.", buffer->length);

    dtlog_info(TAG, "buffered payload is: %s", (char*)buffer->payload);

cleanup:
    // free the buffer
    dtbuffer_dispose(buffer);

    if (dterr != NULL)
    {
        dtlog_dterr(TAG, dterr);
        dterr->dispose(dterr);
        dterr = NULL;
    }
}