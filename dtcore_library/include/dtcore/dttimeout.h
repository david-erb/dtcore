/*
 * dttimeout -- Timeout duration type and sentinel constants.
 *
 * Defines a signed 32-bit millisecond timeout type with FOREVER (-1) and
 * NOWAIT (0) sentinels for use in blocking and polling call sites.
 *
 * cdox v1.0.2
 */
#pragma once

#include <stdint.h>

typedef int32_t dttimeout_millis_t;
#define DTTIMEOUT_MILLIS_PRI PRId32

#define DTTIMEOUT_FOREVER (-1)
#define DTTIMEOUT_NOWAIT (0)
