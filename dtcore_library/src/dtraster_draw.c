#include <stdint.h>

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

#include <dtcore/dtraster.h>
#include <dtcore/dtrgba8888.h>
#include <dtcore/dtvtable.h>

// --------------------------------------------------------------------------------------------
dterr_t*
dtraster_draw_rectangle_fill(dtraster_handle raster_handle, int32_t x, int32_t y, int32_t w, int32_t h, dtrgba8888_t rgba8888)
{
    dterr_t* dterr = NULL;
    dtraster_store_pixel_fn store_pixel_fn = NULL;

    DTVTABLE_GET_FUNCTION(raster_handle, dtraster, store_pixel, store_pixel_fn);

    for (int32_t yy = y; yy < y + h; yy++)
    {
        for (int32_t xx = x; xx < x + w; xx++)
        {
            DTERR_C(store_pixel_fn(raster_handle, xx, yy, rgba8888));
        }
    }

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtraster_draw_rectangle_edge(dtraster_handle raster_handle, int32_t x, int32_t y, int32_t w, int32_t h, dtrgba8888_t rgba8888)
{
    dterr_t* dterr = NULL;
    dtraster_store_pixel_fn store_pixel_fn = NULL;

    DTVTABLE_GET_FUNCTION(raster_handle, dtraster, store_pixel, store_pixel_fn);

    for (int32_t yy = y; yy < y + h; yy++)
    {
        DTERR_C(store_pixel_fn(raster_handle, x, yy, rgba8888));
        DTERR_C(store_pixel_fn(raster_handle, x + w - 1, yy, rgba8888));
    }

    for (int32_t xx = x; xx < x + w; xx++)
    {
        DTERR_C(store_pixel_fn(raster_handle, xx, y, rgba8888));
        DTERR_C(store_pixel_fn(raster_handle, xx, y + h - 1, rgba8888));
    }

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtraster_draw_rectangles_primary(dtraster_handle raster_handle,
  int32_t x,
  int32_t y,
  int32_t w,
  int32_t h,
  int32_t gutter,
  bool vertially,
  dtrgba8888_t rgba8888_edge_color)
{
    dterr_t* dterr = NULL;
    dtrgba8888_t rgba8888_fill_colors[] = { //
        DTRGBA8888_BLACK,
        DTRGBA8888_WHITE,
        DTRGBA8888_RED,
        DTRGBA8888_GREEN,
        DTRGBA8888_BLUE,
        DTRGBA8888_CYAN,
        DTRGBA8888_MAGENTA,
        DTRGBA8888_YELLOW
    };

    int ncolors = sizeof(rgba8888_fill_colors) / sizeof(rgba8888_fill_colors[0]);
    bool should_draw_edge = rgba8888_edge_color.a > 0;

    for (int i = 0; i < ncolors; i++)
    {
        dtrgba8888_t fill_color = rgba8888_fill_colors[i];

        DTERR_C(dtraster_draw_rectangle_fill(raster_handle, x, y, w, h, fill_color));

        if (should_draw_edge)
        {
            DTERR_C(dtraster_draw_rectangle_edge(raster_handle, x, y, w, h, rgba8888_edge_color));
        }

        if (vertially)
        {
            y += h + gutter;
        }
        else
        {
            x += w + gutter;
        }
    }

cleanup:
    return dterr;
}