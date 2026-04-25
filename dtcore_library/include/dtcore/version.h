/*
 * dtcore -- Library flavor and version identification macros.
 *
 * Provides DTCORE_VERSION string constants that identify the library build at runtime.
 * Used by dtruntime and diagnostic routines to report the active library variant in environment tables and
 * log output.
 *
 * This file is processed by tooling in the automated build system.
 * It's important to maintain the structure and formatting of the version macros for compatibility with version parsing scripts.
 *
 * cdox v1.0.2.1
 */

#pragma once

// @dtack-version-file DTCORE

#define DTCORE_VERSION_MAJOR 1
#define DTCORE_VERSION_MINOR 1
#define DTCORE_VERSION_PATCH 15

#define DTCORE_VERSION_STR_(x) #x
#define DTCORE_VERSION_STR(x) DTCORE_VERSION_STR_(x)
#define DTCORE_VERSION                                                                                                         \
    DTCORE_VERSION_STR(DTCORE_VERSION_MAJOR)                                                                                   \
    "." DTCORE_VERSION_STR(DTCORE_VERSION_MINOR) "." DTCORE_VERSION_STR(DTCORE_VERSION_PATCH)