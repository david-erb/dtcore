#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dtbuffer.h>
#include <dtcore/dtcore_constants.h>
#include <dtcore/dterr.h>
#include <dtcore/dtobject.h>
#include <dtcore/dtpackable.h>
#include <dtcore/dtraster.h>
#include <dtcore/dtraster_rgb565.h>
#include <dtcore/dtrgb565.h>
#include <dtcore/dtrgba8888.h>
#include <dtcore/dtunittest.h>

// --------------------------------------------------------------------------------------------
static dterr_t*
_create_raster(dtraster_handle* handle, int32_t w, int32_t h)
{
    dterr_t* dterr = NULL;
    dtraster_rgb565_t* self = NULL;
    DTERR_ASSERT_NOT_NULL(handle);

    DTERR_C(dtraster_rgb565_create(&self));

    dtraster_rgb565_config_t config = {
        .w = w,
        .h = h,
        .stride_bytes = w * sizeof(dtrgb565_t),
    };

    DTERR_C(dtraster_rgb565_config(self, &config));

    DTERR_C(dtraster_rgb565_new_buffer(self));

    *handle = (dtraster_handle)self;

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
static dterr_t*
_fill_raster(dtraster_handle handle, dtrgba8888_t color)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(handle);

    dtraster_shape_t shape;
    DTERR_C(dtraster_get_shape(handle, &shape));

    for (int32_t y = 0; y < shape.h; y++)
    {
        for (int32_t x = 0; x < shape.w; x++)
        {
            DTERR_C(dtraster_store_pixel(handle, x, y, color));
        }
    }

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
// Test store_pixel/fetch_pixel with RGB565 quantization
dterr_t*
test_dtraster_rgb565_store_then_fetch_pixel(void)
{
    dterr_t* dterr = NULL;
    dtraster_handle raster_handle = NULL;
    dtrgba8888_t pixel = { 22, 33, 44, 0xff };
    dtrgba8888_t expected = { 0 };
    dtrgba8888_t actual = { 0 };

    DTERR_C(_create_raster(&raster_handle, 3, 2));

    DTERR_C(dtraster_store_pixel(raster_handle, 1, 1, pixel));
    DTERR_C(dtraster_fetch_pixel(raster_handle, 1, 1, &actual));

    expected = dtrgb565_to_rgba8888(dtrgb565_from_rgba8888(pixel));

    DTUNITTEST_ASSERT_TRUE(dtrgba8888_compare(expected, actual) == 0);

cleanup:
    dtraster_dispose(raster_handle);

    return dterr;
}

// --------------------------------------------------------------------------------------------
// Test blit in the middle
dterr_t*
test_dtraster_rgb565_blit_middle(void)
{
    dterr_t* dterr = NULL;
    dtraster_handle raster1_handle = NULL;
    dtraster_handle raster2_handle = NULL;
    dtraster_handle raster3_handle = NULL;
    dtrgba8888_t pixel1 = DTRGBA8888_DARKGRAY;
    dtrgba8888_t pixel2 = DTRGBA8888_TURQUOISE;

    DTERR_C(_create_raster(&raster1_handle, 5, 5));
    DTERR_C(_create_raster(&raster2_handle, 3, 3));
    DTERR_C(_create_raster(&raster3_handle, 5, 5));
    DTERR_C(_fill_raster(raster1_handle, pixel1));
    DTERR_C(_fill_raster(raster2_handle, pixel2));

    DTERR_C(dtraster_blit(raster1_handle, raster2_handle, 1, 1));

    // expected result
    DTERR_C(_fill_raster(raster3_handle, pixel1));
    for (int32_t y = 1; y <= 3; y++)
    {
        for (int32_t x = 1; x <= 3; x++)
        {
            DTERR_C(dtraster_store_pixel(raster3_handle, x, y, pixel2));
        }
    }

    DTUNITTEST_ASSERT_TRUE(dtobject_equals((dtobject_handle)raster1_handle, (dtobject_handle)raster3_handle));

cleanup:
    dtraster_dispose(raster1_handle);
    dtraster_dispose(raster2_handle);
    dtraster_dispose(raster3_handle);

    return dterr;
}

// --------------------------------------------------------------------------------------------
// Test blit clipped on the right and bottom.
dterr_t*
test_dtraster_rgb565_blit_clip_right_bottom(void)
{
    dterr_t* dterr = NULL;
    dtraster_handle raster1_handle = NULL;
    dtraster_handle raster2_handle = NULL;
    dtraster_handle raster3_handle = NULL;
    dtrgba8888_t pixel1 = DTRGBA8888_OLIVE;
    dtrgba8888_t pixel2 = DTRGBA8888_ORANGE;

    DTERR_C(_create_raster(&raster1_handle, 5, 5));
    DTERR_C(_create_raster(&raster2_handle, 3, 3));
    DTERR_C(_create_raster(&raster3_handle, 5, 5));
    DTERR_C(_fill_raster(raster1_handle, pixel1));
    DTERR_C(_fill_raster(raster2_handle, pixel2));

    DTERR_C(dtraster_blit(raster1_handle, raster2_handle, 3, 3));

    // expected result
    DTERR_C(_fill_raster(raster3_handle, pixel1));
    for (int32_t y = 3; y <= 4; y++)
    {
        for (int32_t x = 3; x <= 4; x++)
        {
            DTERR_C(dtraster_store_pixel(raster3_handle, x, y, pixel2));
        }
    }

    DTUNITTEST_ASSERT_TRUE(dtobject_equals((dtobject_handle)raster1_handle, (dtobject_handle)raster3_handle));

cleanup:
    dtraster_dispose(raster1_handle);
    dtraster_dispose(raster2_handle);
    dtraster_dispose(raster3_handle);

    return dterr;
}

// --------------------------------------------------------------------------------------------
// Test blit clipped on the left and top.
dterr_t*
test_dtraster_rgb565_blit_clip_left_top(void)
{
    dterr_t* dterr = NULL;
    dtraster_handle raster1_handle = NULL;
    dtraster_handle raster2_handle = NULL;
    dtraster_handle raster3_handle = NULL;
    dtrgba8888_t pixel1 = DTRGBA8888_NAVY;
    dtrgba8888_t pixel2 = DTRGBA8888_GOLD;

    DTERR_C(_create_raster(&raster1_handle, 5, 5));
    DTERR_C(_create_raster(&raster2_handle, 3, 3));
    DTERR_C(_create_raster(&raster3_handle, 5, 5));
    DTERR_C(_fill_raster(raster1_handle, pixel1));
    DTERR_C(_fill_raster(raster2_handle, pixel2));

    DTERR_C(dtraster_blit(raster1_handle, raster2_handle, -1, -1));

    // expected result
    DTERR_C(_fill_raster(raster3_handle, pixel1));
    for (int32_t y = 0; y <= 1; y++)
    {
        for (int32_t x = 0; x <= 1; x++)
        {
            DTERR_C(dtraster_store_pixel(raster3_handle, x, y, pixel2));
        }
    }

    DTUNITTEST_ASSERT_TRUE(dtobject_equals((dtobject_handle)raster1_handle, (dtobject_handle)raster3_handle));

cleanup:
    dtraster_dispose(raster1_handle);
    dtraster_dispose(raster2_handle);
    dtraster_dispose(raster3_handle);

    return dterr;
}

// --------------------------------------------------------------------------------------------
// suite runner
void
test_dtraster_rgb565(DTUNITTEST_SUITE_ARGS)
{
    DTUNITTEST_RUN_TEST(test_dtraster_rgb565_store_then_fetch_pixel);
    DTUNITTEST_RUN_TEST(test_dtraster_rgb565_blit_middle);
    DTUNITTEST_RUN_TEST(test_dtraster_rgb565_blit_clip_right_bottom);
    DTUNITTEST_RUN_TEST(test_dtraster_rgb565_blit_clip_left_top);
}