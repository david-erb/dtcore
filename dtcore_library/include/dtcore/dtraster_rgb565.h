/*
 * dtraster_rgb565 -- RGB565 raster implementation with optional big-endian storage.
 *
 * Provides a pixel buffer in 16-bit RGB565 format, implementing the dtraster,
 * dtobject, and dtpackable facades.  Configurable byte order allows targeting
 * displays that expect big-endian packed pixels alongside the default
 * little-endian layout.
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
#include <dtcore/dtrgb565.h>

// configuration for this implementation
typedef struct dtraster_rgb565_config_t
{
    int32_t w;
    int32_t h;
    int32_t stride_bytes;
    bool should_store_as_big_endian;
} dtraster_rgb565_config_t;

typedef struct dtraster_rgb565_t dtraster_rgb565_t;

dterr_t*
dtraster_rgb565_create(dtraster_rgb565_t** this);
dterr_t*
dtraster_rgb565_init(dtraster_rgb565_t* this);
dterr_t*
dtraster_rgb565_config(dtraster_rgb565_t* this, dtraster_rgb565_config_t* configuration);
dterr_t*
dtraster_rgb565_register_vtables(void);

dterr_t*
dtraster_rgb565_blit(dtraster_rgb565_t* self DTRASTER_BLIT_ARGS);
dterr_t*
dtraster_rgb565_create_compatible_raster(dtraster_rgb565_t* self DTRASTER_CREATE_COMPATIBLE_RASTER_ARGS);
dterr_t*
dtraster_rgb565_use_buffer(dtraster_rgb565_t* self DTRASTER_USE_BUFFER_ARGS);
dterr_t*
dtraster_rgb565_new_buffer(dtraster_rgb565_t* self DTRASTER_NEW_BUFFER_ARGS);
dterr_t*
dtraster_rgb565_fetch_pixel(dtraster_rgb565_t* self DTRASTER_FETCH_PIXEL_ARGS);
dterr_t*
dtraster_rgb565_store_pixel(dtraster_rgb565_t* self DTRASTER_STORE_PIXEL_ARGS);

// --------------------------------------------------------------------------------------------

// facade implementations
DTRASTER_DECLARE_API(dtraster_rgb565);
DTOBJECT_DECLARE_API(dtraster_rgb565);
DTPACKABLE_DECLARE_API(dtraster_rgb565);
