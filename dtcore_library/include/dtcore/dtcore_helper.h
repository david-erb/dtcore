/*
 * dtcore_helper -- Saturating integer arithmetic macros.
 *
 * Provides in-place increment and add macros for int32_t counters that
 * clamp at INT32_MAX rather than wrapping, suitable for monotonic counters
 * where overflow would corrupt state.
 *
 * cdox v1.0.2
 */
#pragma once

// increment int32_t counter with saturation at INT32_MAX
#define DTCORE_HELPER_INC32(X) ((X) += ((X) < INT32_MAX) ? 1 : 0)

// add delta to int32_t counter with saturation at INT32_MAX
#define DTCORE_HELPER_ADD32(X, DELTA) ((X) += ((X) <= INT32_MAX - (DELTA)) ? (DELTA) : (INT32_MAX - (X)))