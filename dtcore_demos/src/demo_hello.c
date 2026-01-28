/*
demo_hello: Minimal example

This demo establishes the smallest possible dtcore application.

It's intended as a fast sanity check on a new platform.

The output you'd expect to see when running this demo is:

    [INFO ] dtcore_demo_hello: hello from dtcore!

The demo_hello app runs this demo automatically.

Related modules: dtlog
*/

#include <dtcore/dtlog.h>

#define TAG "dtcore_demo_hello"

// ------------------------------------------------------------------------
void
dtcore_demo_hello(void)
{
    dtlog_info(TAG, "hello from dtcore!");
}
