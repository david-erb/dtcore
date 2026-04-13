/*
 * dtglyph -- Vtable facade for text glyph rendering.
 *
 * Defines a two-method dispatch interface for blitting a text string onto
 * a raster at a given position with a color, and for computing the bounding
 * box of a string.  Concrete implementations register a vtable keyed by
 * model number for selection at runtime.
 *
 * cdox v1.0.2
 */
#pragma once

#include <stdint.h>

#include <dtcore/dtbuffer.h>

#include <dtcore/dterr.h>
#include <dtcore/dtobject.h>

#include <dtcore/dtglyph.h>
#include <dtcore/dtraster.h>
#include <dtcore/dtrgba8888.h>

// opaque handle for dispatch calls
struct dtglyph_handle_t;
typedef struct dtglyph_handle_t* dtglyph_handle;

// arguments
#define DTGLYPH_BLIT_ARGS , const char *text, dtraster_handle raster_handle, int32_t x, int32_t y, dtrgba8888_t rgba8888
#define DTGLYPH_CALCULATE_BOX_ARGS , const char *text, int32_t *w, int32_t *h

#define DTGLYPH_BLIT_PARAMS , text, raster_handle, x, y, rgba8888
#define DTGLYPH_CALCULATE_BOX_PARAMS , text, w, h

// delegates
typedef dterr_t* (*dtglyph_blit_fn)(void* self DTGLYPH_BLIT_ARGS);
typedef dterr_t* (*dtglyph_calculate_box_fn)(void* self DTGLYPH_CALCULATE_BOX_ARGS);
typedef void (*dtglyph_dispose_fn)(void* self);

// virtual table type
typedef struct dtglyph_vt_t
{
    dtglyph_blit_fn blit;
    dtglyph_calculate_box_fn calculate_box;
    dtglyph_dispose_fn dispose;

} dtglyph_vt_t;

// vtable registration
extern dterr_t*
dtglyph_set_vtable(int32_t model_number, dtglyph_vt_t* vtable);

extern dterr_t*
dtglyph_get_vtable(int32_t model_number, dtglyph_vt_t** vtable);

// declaration dispatcher or implementation
#define DTGLYPH_DECLARE_API_EX(NAME, T)                                                                                        \
    extern dterr_t* NAME##_blit(NAME##T self DTGLYPH_BLIT_ARGS);                                                               \
    extern dterr_t* NAME##_calculate_box(NAME##T self DTGLYPH_CALCULATE_BOX_ARGS);                                             \
    extern void NAME##_dispose(NAME##T self);

// declare dispatcher
DTGLYPH_DECLARE_API_EX(dtglyph, _handle)

// declare implementation (put this in its .h file)
#define DTGLYPH_DECLARE_API(NAME) DTGLYPH_DECLARE_API_EX(NAME, _t*)

// initialize implementation vtable (put this in its .c file)
#define DTGLYPH_INIT_VTABLE(NAME)                                                                                              \
    static dtglyph_vt_t NAME##_glyph_vt = {                                                                                    \
        .blit = (dtglyph_blit_fn)NAME##_blit,                                                                                  \
        .calculate_box = (dtglyph_calculate_box_fn)NAME##_calculate_box,                                                       \
        .dispose = (dtglyph_dispose_fn)NAME##_dispose,                                                                         \
    };

// common members expected at the start of all implementation structures
#define DTGLYPH_COMMON_MEMBERS int32_t model_number;
