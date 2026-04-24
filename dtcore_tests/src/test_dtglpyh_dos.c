#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dtbuffer.h>
#include <dtcore/dtcore_constants.h>

#include <dtcore/dterr.h>
#include <dtcore/dtglyph.h>
#include <dtcore/dtglyph_dos.h>
#include <dtcore/dtobject.h>
#include <dtcore/dtpackable.h>
#include <dtcore/dtraster.h>
#include <dtcore/dtraster_rgba8888.h>
#include <dtcore/dtrgba8888.h>
#include <dtcore/dtunittest.h>

// --------------------------------------------------------------------------------------------
static dterr_t*
_create_raster(dtraster_handle* handle, int32_t w, int32_t h)
{
    dterr_t* dterr = NULL;
    dtraster_rgba8888_t* self = NULL;
    DTERR_ASSERT_NOT_NULL(handle);

    DTERR_C(dtraster_rgba8888_create(&self));

    dtraster_rgba8888_config_t config = {
        .w = w,
        .h = h,
        .stride_bytes = w * (int32_t)sizeof(dtrgba8888_t),
    };

    DTERR_C(dtraster_rgba8888_config(self, &config));

    DTERR_C(dtraster_rgba8888_new_buffer(self));

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

    for (int y = 0; y < shape.h; y++)
    {
        for (int x = 0; x < shape.w; x++)
        {
            DTERR_C(dtraster_store_pixel(handle, x, y, color));
        }
    }

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
static dterr_t*
_assert_pixel_equals(dtraster_handle handle, int32_t x, int32_t y, dtrgba8888_t expected)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(handle);
    dtrgba8888_t actual = { 0 };
    DTERR_C(dtraster_fetch_pixel(handle, x, y, &actual));

    DTUNITTEST_ASSERT_INT(actual.r, ==, expected.r);
    DTUNITTEST_ASSERT_INT(actual.g, ==, expected.g);
    DTUNITTEST_ASSERT_INT(actual.b, ==, expected.b);
    DTUNITTEST_ASSERT_INT(actual.a, ==, expected.a);
cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
static dterr_t*
_assert_pattern(dtraster_handle handle,
  const char* rows[],
  int32_t expected_w,
  int32_t expected_h,
  dtrgba8888_t foreground,
  dtrgba8888_t background)
{
    dterr_t* dterr = NULL;
    dtraster_shape_t shape;

    DTERR_C(dtraster_get_shape(handle, &shape));

    DTUNITTEST_ASSERT_INT(shape.w, ==, expected_w);
    DTUNITTEST_ASSERT_INT(shape.h, ==, expected_h);

    for (int32_t y = 0; y < expected_h; y++)
    {
        for (int32_t x = 0; x < expected_w; x++)
        {
            dtrgba8888_t expected = rows[y][x] == '#' ? foreground : background;
            DTERR_C(_assert_pixel_equals(handle, x, y, expected));
        }
    }
cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtglyph_dos__dump_raster(dtraster_handle handle, dtrgba8888_t foreground)
{
    dterr_t* dterr = NULL;
    dtraster_shape_t shape;
    DTERR_C(dtraster_get_shape(handle, &shape));

    for (int y = 0; y < shape.h; y++)
    {
        putchar('"');
        for (int x = 0; x < shape.w; x++)
        {
            dtrgba8888_t pixel = { 0 };
            DTERR_C(dtraster_fetch_pixel(handle, x, y, &pixel));

            bool on = pixel.r == foreground.r && pixel.g == foreground.g && pixel.b == foreground.b && pixel.a == foreground.a;

            putchar(on ? '#' : '.');
        }
        putchar('"');
        putchar(',');
        putchar('\n');
    }
cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtglyph_dos_single(void)
{
    dterr_t* dterr = NULL;
    dtglyph_dos_t* glyph_object = NULL;
    dtglyph_handle glyph_handle = NULL;
    dtraster_handle raster_handle = NULL;
    dtrgba8888_t background_color = { 0x00, 0x00, 0x00, 0x00 };
    dtrgba8888_t foreground_color = { 0xff, 0xff, 0xff, 0xff };

    DTERR_C(dtglyph_dos_create(&glyph_object));
    glyph_handle = (dtglyph_handle)glyph_object;

    int32_t w, h;
    DTERR_C(dtglyph_calculate_box(glyph_handle, "A", &w, &h));

    DTUNITTEST_ASSERT_INT(w, ==, 8);
    DTUNITTEST_ASSERT_INT(h, ==, 16);

    DTERR_C(_create_raster(&raster_handle, w, h));
    DTERR_C(_fill_raster(raster_handle, background_color));
    DTERR_C(dtglyph_blit(glyph_handle, "A", raster_handle, 0, 0, foreground_color));

    /* Replace these rows with the actual expected A shape from your font. */
    const char* expected[16] = {
        "........",
        "...#....",
        "..###...",
        ".##.##..",
        "##...##.",
        "##...##.",
        "#######.",
        "##...##.",
        "##...##.",
        "##...##.",
        "##...##.",
        "........",
        "........",
        "........",
        "........",
        "........",

    };

    DTERR_C(_assert_pattern(raster_handle, expected, 8, 16, foreground_color, background_color));

    DTERR_C(_fill_raster(raster_handle, background_color));
    DTERR_C(dtglyph_blit(glyph_handle, "B", raster_handle, 0, 0, foreground_color));

cleanup:
    dtraster_dispose(raster_handle);
    dtglyph_dispose(glyph_handle);

    return dterr;
}
// --------------------------------------------------------------------------------------------
dterr_t*
test_dtglyph_dos_multiline(void)
{
    dterr_t* dterr = NULL;
    dtglyph_dos_t* glyph_object = NULL;
    dtglyph_handle glyph_handle = NULL;
    dtraster_handle raster_handle = NULL;
    dtrgba8888_t background_color = { 0x00, 0x00, 0x00, 0x00 };
    dtrgba8888_t foreground_color = { 0xff, 0xff, 0xff, 0xff };

    DTERR_C(dtglyph_dos_create(&glyph_object));
    glyph_handle = (dtglyph_handle)glyph_object;

    int32_t w, h;
    const char* text = "AB\nCD";
    DTERR_C(dtglyph_calculate_box(glyph_handle, text, &w, &h));

    DTUNITTEST_ASSERT_INT(w, ==, 16);
    DTUNITTEST_ASSERT_INT(h, ==, 32);

    DTERR_C(_create_raster(&raster_handle, w, h));
    DTERR_C(_fill_raster(raster_handle, background_color));
    DTERR_C(dtglyph_blit(glyph_handle, text, raster_handle, 0, 0, foreground_color));

    const char* expected[32] = { "................",
        "...#....######..",
        "..###....##..##.",
        ".##.##...##..##.",
        "##...##..##..##.",
        "##...##..#####..",
        "#######..##..##.",
        "##...##..##..##.",
        "##...##..##..##.",
        "##...##..##..##.",
        "##...##.######..",
        "................",
        "................",
        "................",
        "................",
        "................",
        "................",
        "..####..#####...",
        ".##..##..##.##..",
        "##....#..##..##.",
        "##.......##..##.",
        "##.......##..##.",
        "##.......##..##.",
        "##.......##..##.",
        "##....#..##..##.",
        ".##..##..##.##..",
        "..####..#####...",
        "................",
        "................",
        "................",
        "................",
        "................" };

    DTERR_C(_assert_pattern(raster_handle, expected, 16, 32, foreground_color, background_color));

cleanup:
    dtraster_dispose(raster_handle);
    dtglyph_dispose(glyph_handle);

    return dterr;
}

// --------------------------------------------------------------------------------------------
void
test_dtglyph_dos(DTUNITTEST_SUITE_ARGS)
{
    DTUNITTEST_RUN_TEST(test_dtglyph_dos_single);
    DTUNITTEST_RUN_TEST(test_dtglyph_dos_multiline);
}