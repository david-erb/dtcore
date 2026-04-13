/*
 * dtwallclock -- Wallclock duration formatting helpers.
 *
 * Formats integer microsecond or millisecond durations into fixed-width
 * HH:MM:SS strings suitable for log output and diagnostics.  Avoids
 * dynamic allocation by writing into caller-provided storage.
 *
 * cdox v1.0.2
 */
#pragma once

#include <dtcore/dterr.h>

// -------------------------------------------------------------------------------
extern dterr_t*
dtwallclock_format_microseconds_as_hhmmss_llluuu(uint64_t timestamp_micros,
                                                 char* buffer,
                                                 size_t buffer_size);

extern dterr_t*
dtwallclock_format_milliseconds_as_hhmmss_lll(uint64_t timestamp_millis,
                                              char* buffer,
                                              size_t buffer_size);

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtwallclock

Format wallclock durations into human-readable timestamps.

This group of functions provides timestamp formatting for wallclock-style durations.
It is mainly for diagnostics, logging, and presentation paths that require fixed-width time strings.
The formatting does not include localization.

## Mini-guide

- Formats integer microsecond or millisecond durations into fixed-width strings.
- Uses 24-hour style hours without rollover handling beyond the provided duration.
- Avoids dynamic allocation by relying entirely on caller-owned storage.

## Example

```c
char buf[32];
dterr_t* err;

err = dtwallclock_format_microseconds_as_hhmmss_llluuu(3723004005ULL, buf, sizeof(buf));
/* buf -> "01:02:03 004.005" */

err = dtwallclock_format_milliseconds_as_hhmmss_lll(3723004ULL, buf, sizeof(buf));
/* buf -> "01:02:03.004" */
```

## Data structures

## Functions

### dtwallclock_format_microseconds_as_hhmmss_llluuu

Formats a microsecond duration into an `HH:MM:SS mmm.uuu` string.

Params:

> `uint64_t timestamp_micros`  Duration in microseconds to format.  
> `char* buffer`  Destination buffer for the formatted string.  
> `size_t buffer_size`  Size of the destination buffer in bytes.  

Return: `dterr_t*`  Error object when arguments are invalid, otherwise NULL.

### dtwallclock_format_milliseconds_as_hhmmss_lll

Formats a millisecond duration into an `HH:MM:SS.mmm` string.

Params:

> `uint64_t timestamp_millis`  Duration in milliseconds to format.  
> `char* buffer`  Destination buffer for the formatted string.  
> `size_t buffer_size`  Size of the destination buffer in bytes.  

Return: `dterr_t*`  Error object when arguments are invalid, otherwise NULL.

<!-- FG_IDC: f34bc05c-b49f-49f0-9963-9c843a8556ac | FG_UTC: 2026-01-18T09:08:37Z | FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
