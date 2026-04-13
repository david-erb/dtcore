#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dtcore_constants.h>

#include <dtcore/dterr.h>
#include <dtcore/dtheaper.h>
#include <dtcore/dtlog.h>
#include <dtcore/dtobject.h>

#include <dtcore/dtglyph.h>
#include <dtcore/dtglyph_dos.h>
#include <dtcore/dtraster.h>

DTGLYPH_INIT_VTABLE(dtglyph_dos);
DTOBJECT_INIT_VTABLE(dtglyph_dos);

#define TAG "dtglyph_dos"

// the implementation
typedef struct dtglyph_dos_t
{
    DTGLYPH_COMMON_MEMBERS;
    bool is_malloced;
} dtglyph_dos_t;

// --------------------------------------------------------------------------------------------
dterr_t*
dtglyph_dos_create(dtglyph_dos_t** self)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);

    DTERR_C(dtheaper_alloc_and_zero(sizeof(dtglyph_dos_t), "dtglyph_dos_t", (void**)self));
    DTERR_C(dtglyph_dos_init(*self));

    (*self)->is_malloced = true;

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
static bool vtables_are_registered = false;

dterr_t*
dtglyph_dos_register_vtables(void)
{
    dterr_t* dterr = NULL;

    if (!vtables_are_registered)
    {
        int32_t model_number = DTCORE_CONSTANTS_GLYPH_MODEL_DOS;

        DTERR_C(dtglyph_set_vtable(model_number, &dtglyph_dos_glyph_vt));
        DTERR_C(dtobject_set_vtable(model_number, &dtglyph_dos_object_vt));

        vtables_are_registered = true;
    }

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtglyph_dos_init(dtglyph_dos_t* self)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);

    memset(self, 0, sizeof(*self));
    self->model_number = DTCORE_CONSTANTS_GLYPH_MODEL_DOS;

    DTERR_C(dtglyph_dos_register_vtables());

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtglyph_dos_config(dtglyph_dos_t* self, dtglyph_dos_config_t* configuration)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);

    (void)configuration;

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
typedef struct
{
    const char* order;   /* character order string              */
    const uint8_t* font; /* bitmap table                        */
    int w;               /* characters per row in bitmap table  */
    int dx;              /* character width in bits             */
    int dy;              /* character height in rows            */
} dtfont_fixed_t;

#define ORDER1                                                                                                                 \
    "\
abcdefghijklm\
nopqrstuvwxyz\
ABCDEFGHIJKLM\
NOPQRSTUVWXYZ\
0123456789.:_\
-+=!@#$%^&*()\
 ~{}[]'\"<>,/?\
"

#include "dtglyph_dos_table.h"

/* one fixed font for now */
static const dtfont_fixed_t desc1 = { ORDER1, dtglyph_dos_table_font1, 13, 8, 16 };

// --------------------------------------------------------------------------------------------
static dterr_t*
dtglyph_dos_blit_char(dtglyph_dos_t* self, char c, dtraster_handle raster_handle, int32_t x, int32_t y, dtrgba8888_t rgba8888)
{
    dterr_t* dterr = NULL;
    const char* order_pos;
    const uint8_t* glyph_row;
    int offset;
    int cx, cy;
    int bytes_per_glyph_row;
    int bytes_per_font_row;
    int dx;
    int dy;

    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(raster_handle);

    dx = desc1.dx;
    dy = desc1.dy;
    bytes_per_glyph_row = (dx + 7) / 8;
    bytes_per_font_row = desc1.w * bytes_per_glyph_row;

    order_pos = strchr(desc1.order, c);
    if (order_pos == NULL)
    {
        return dterr_new(DTERR_NOTFOUND, DTERR_LOC, NULL, "character 0x%02x not found in font", (unsigned char)c);
    }

    offset = (int)(order_pos - desc1.order);

    glyph_row = desc1.font;
    glyph_row += (offset / desc1.w) * bytes_per_font_row * dy;
    glyph_row += (offset % desc1.w) * bytes_per_glyph_row;

    for (cy = 0; cy < dy; cy++)
    {
        const uint8_t* p = glyph_row + (cy * bytes_per_font_row);
        uint8_t bits = *p;
        uint8_t mask = 0x80;

        for (cx = 0; cx < dx; cx++)
        {
            if (bits & mask)
            {
                DTERR_C(dtraster_store_pixel(raster_handle, x + cx, y + cy, rgba8888));
            }

            mask >>= 1;
            if (mask == 0)
            {
                p++;
                bits = *p;
                mask = 0x80;
            }
        }
    }

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtglyph_dos_blit(dtglyph_dos_t* self DTGLYPH_BLIT_ARGS)
{
    dterr_t* dterr = NULL;
    int32_t x0;

    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(text);
    DTERR_ASSERT_NOT_NULL(raster_handle);

    x0 = x;

    for (const char* p = text; *p; p++)
    {
        if (*p == '\n')
        {
            x = x0;
            y += desc1.dy;
            continue;
        }

        DTERR_C(dtglyph_dos_blit_char(self, *p, raster_handle, x, y, rgba8888));

        x += desc1.dx;
    }

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtglyph_dos_calculate_box(dtglyph_dos_t* self DTGLYPH_CALCULATE_BOX_ARGS)
{
    dterr_t* dterr = NULL;
    int rows = 1;
    int chars_this_row = 0;
    int max_chars = 0;

    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(text);
    DTERR_ASSERT_NOT_NULL(w);
    DTERR_ASSERT_NOT_NULL(h);

    for (const char* p = text; *p; p++)
    {
        if (*p == '\n')
        {
            if (chars_this_row > max_chars)
                max_chars = chars_this_row;

            rows++;
            chars_this_row = 0;
        }
        else if (*p != '\r')
        {
            chars_this_row++;
        }
    }

    if (chars_this_row > max_chars)
        max_chars = chars_this_row;

    *w = max_chars * desc1.dx;
    *h = rows * desc1.dy;

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
// dtobject implementation
// --------------------------------------------------------------------------------------------
void
dtglyph_dos_copy(dtglyph_dos_t* self, dtglyph_dos_t* that_handle)
{
    // no fields to copy
}

// --------------------------------------------------------------------------------------------
void
dtglyph_dos_dispose(dtglyph_dos_t* self)
{
    if (self == NULL)
        return;

    if (self->is_malloced)
        dtheaper_free(self);
    else
        memset(self, 0, sizeof(*self));
}

// --------------------------------------------------------------------------------------------
bool
dtglyph_dos_equals(dtglyph_dos_t* a, dtglyph_dos_t* b)
{
    return false;
}

// --------------------------------------------------------------------------------------------
const char*
dtglyph_dos_get_class(dtglyph_dos_t* self)
{
    (void)self;
    return "dtglyph_dos_t";
}

// --------------------------------------------------------------------------------------------
bool
dtglyph_dos_is_iface(dtglyph_dos_t* self, const char* iface_name)
{
    (void)self;

    if (iface_name == NULL)
        return false;

    return strcmp(iface_name, "dtglyph_iface") == 0 || //
           strcmp(iface_name, "dtobject_iface") == 0;
}

// --------------------------------------------------------------------------------------------
void
dtglyph_dos_to_string(dtglyph_dos_t* self, char* buffer, size_t buffer_size)
{
    (void)self;
    snprintf(buffer, buffer_size, "dtglyph_dos_t");
}