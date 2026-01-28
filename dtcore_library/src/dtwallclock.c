
#include <inttypes.h>
#include <stdio.h>

#include <dtcore/dterr.h>
#include <dtcore/dtlog.h>

#include <dtcore/dtwallclock.h>

// -------------------------------------------------------------------------------
dterr_t*
dtwallclock_format_microseconds_as_hhmmss_llluuu(uint64_t timestamp_micros, char* buffer, size_t buffer_size)
{
    dterr_t* dterr = NULL;
    if (buffer == NULL || buffer_size == 0)
    {
        dterr = dterr_new(DTERR_ARGUMENT_NULL, DTERR_LOC, NULL, "buffer is NULL or size is 0");
        goto cleanup;
    }

    uint64_t timestamp = timestamp_micros;

    int micros = timestamp % 1000;

    timestamp /= 1000; // convert to milliseconds
    int millis = timestamp % 1000;
    timestamp /= 1000; // convert to seconds
    int seconds = timestamp % 60;
    timestamp /= 60; // convert to minutes
    int minutes = timestamp % 60;
    timestamp /= 60; // convert to hours
    int hours = timestamp;

    snprintf(buffer, buffer_size, "%02d:%02d:%02d %03d.%03d", hours, minutes, seconds, millis, micros);

cleanup:
    return dterr;
}

// -------------------------------------------------------------------------------
dterr_t*
dtwallclock_format_milliseconds_as_hhmmss_lll(uint64_t timestamp_millis, char* buffer, size_t buffer_size)
{
    dterr_t* dterr = NULL;
    if (buffer == NULL || buffer_size == 0)
    {
        dterr = dterr_new(DTERR_ARGUMENT_NULL, DTERR_LOC, NULL, "buffer is NULL or size is 0");
        goto cleanup;
    }

    uint64_t timestamp = timestamp_millis;

    int millis = timestamp % 1000;
    timestamp /= 1000; // convert to seconds
    int seconds = timestamp % 60;
    timestamp /= 60; // convert to minutes
    int minutes = timestamp % 60;
    timestamp /= 60; // convert to hours
    int hours = timestamp;

    snprintf(buffer, buffer_size, "%02d:%02d:%02d.%03d", hours, minutes, seconds, millis);

cleanup:
    return dterr;
}