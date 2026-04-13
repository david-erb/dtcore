/*
 * dtglyph_dos -- DOS-style fixed-width glyph renderer.
 *
 * Implements the dtglyph facade for blitting text and computing bounding
 * boxes using a DOS-style fixed-width font.  Configurable width, height,
 * and stride allow the renderer to target different raster formats.
 *
 * cdox v1.0.2
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <dtcore/dterr.h>
#include <dtcore/dtglyph.h>
#include <dtcore/dtobject.h>

// configuration for this implementation
typedef struct dtglyph_dos_config_t
{
    uint16_t w;
    uint16_t h;
    uint16_t stride_bytes;
} dtglyph_dos_config_t;

typedef struct dtglyph_dos_t dtglyph_dos_t;

dterr_t*
dtglyph_dos_create(dtglyph_dos_t** this);
dterr_t*
dtglyph_dos_init(dtglyph_dos_t* this);
dterr_t*
dtglyph_dos_config(dtglyph_dos_t* this, dtglyph_dos_config_t* configuration);
dterr_t*
dtglyph_dos_register_vtables(void);

dterr_t*
dtglyph_dos_blit(dtglyph_dos_t* self DTGLYPH_BLIT_ARGS);
dterr_t*
dtglyph_dos_calculate_box(dtglyph_dos_t* self DTGLYPH_CALCULATE_BOX_ARGS);

// --------------------------------------------------------------------------------------------

// facade implementations
DTGLYPH_DECLARE_API(dtglyph_dos);
DTOBJECT_DECLARE_API(dtglyph_dos);
