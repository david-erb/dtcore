/*
 * dtparse -- Strict decimal numeric parsing with full-input validation.
 *
 * Parses signed 32-bit integers and double-precision floats from C strings,
 * rejecting trailing characters, partial conversions, out-of-range values,
 * and non-finite results.  Addresses a common gap in libc parsers, which
 * accept trailing garbage and report success.
 *
 * cdox v1.0.2
 */
#pragma once

#include <limits.h>
#include <stdbool.h>
#include <stdint.h>

extern bool
dtparse_int32(const char* string, int32_t* out_value);

extern bool
dtparse_double(const char* string, double* out_value);