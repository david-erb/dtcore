/*
 * dtraster_rgba8888 -- RGBA8888 raster implementation.
 *
 * Provides a heap-backed pixel buffer in 32-bit RGBA8888 format, implementing
 * the dtraster, dtobject, and dtpackable facades.  Supports blit, per-pixel
 * read/write, buffer assignment, and new-buffer allocation.
 *
 * cdox v1.0.2
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <dtcore/dterr.h>
#include <dtcore/dtobject.h>
#include <dtcore/dtpackable.h>
#include <dtcore/dtraster.h>
#include <dtcore/dtrgba8888.h>

// configuration for this implementation
typedef struct dtraster_rgba8888_config_t
{
    int32_t w;
    int32_t h;
    int32_t stride_bytes;
} dtraster_rgba8888_config_t;

typedef struct dtraster_rgba8888_t dtraster_rgba8888_t;

dterr_t*
dtraster_rgba8888_create(dtraster_rgba8888_t** this);
dterr_t*
dtraster_rgba8888_init(dtraster_rgba8888_t* this);
dterr_t*
dtraster_rgba8888_config(dtraster_rgba8888_t* this, dtraster_rgba8888_config_t* configuration);
dterr_t*
dtraster_rgba8888_register_vtables(void);

dterr_t*
dtraster_rgba8888_blit(dtraster_rgba8888_t* self DTRASTER_BLIT_ARGS);
dterr_t*
dtraster_rgba8888_use_buffer(dtraster_rgba8888_t* self DTRASTER_USE_BUFFER_ARGS);
dterr_t*
dtraster_rgba8888_new_buffer(dtraster_rgba8888_t* self DTRASTER_NEW_BUFFER_ARGS);
dterr_t*
dtraster_rgba8888_fetch_pixel(dtraster_rgba8888_t* self DTRASTER_FETCH_PIXEL_ARGS);
dterr_t*
dtraster_rgba8888_store_pixel(dtraster_rgba8888_t* self DTRASTER_STORE_PIXEL_ARGS);

// --------------------------------------------------------------------------------------------

// facade implementations
DTRASTER_DECLARE_API(dtraster_rgba8888);
DTOBJECT_DECLARE_API(dtraster_rgba8888);
DTPACKABLE_DECLARE_API(dtraster_rgba8888);
