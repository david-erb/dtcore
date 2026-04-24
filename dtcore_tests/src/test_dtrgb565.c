#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dterr.h>
#include <dtcore/dtrgb565.h>
#include <dtcore/dtrgba8888.h>
#include <dtcore/dtunittest.h>

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_constructor_black(void)
{
    dterr_t* dterr = NULL;
    dtrgb565_t c = dtrgb565(0, 0, 0);

    DTUNITTEST_ASSERT_TRUE(c.value == 0x0000);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_constructor_white(void)
{
    dterr_t* dterr = NULL;
    dtrgb565_t c = dtrgb565(255, 255, 255);

    DTUNITTEST_ASSERT_TRUE(c.value == 0xFFFF);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_constructor_red(void)
{
    dterr_t* dterr = NULL;
    dtrgb565_t c = dtrgb565(255, 0, 0);

    DTUNITTEST_ASSERT_TRUE(c.value == 0xF800);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_constructor_green(void)
{
    dterr_t* dterr = NULL;
    dtrgb565_t c = dtrgb565(0, 255, 0);

    DTUNITTEST_ASSERT_TRUE(c.value == 0x07E0);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_constructor_blue(void)
{
    dterr_t* dterr = NULL;
    dtrgb565_t c = dtrgb565(0, 0, 255);

    DTUNITTEST_ASSERT_TRUE(c.value == 0x001F);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_raw_to_rgba8888_black(void)
{
    dterr_t* dterr = NULL;
    dtrgb565_t c = dtrgb565_raw(0x0000);
    dtrgba8888_t rgba = dtrgb565_to_rgba8888(c);

    DTUNITTEST_ASSERT_TRUE(rgba.r == 0);
    DTUNITTEST_ASSERT_TRUE(rgba.g == 0);
    DTUNITTEST_ASSERT_TRUE(rgba.b == 0);
    DTUNITTEST_ASSERT_TRUE(rgba.a == 0xFF);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_raw_to_rgba8888_white(void)
{
    dterr_t* dterr = NULL;
    dtrgb565_t c = dtrgb565_raw(0xFFFF);
    dtrgba8888_t rgba = dtrgb565_to_rgba8888(c);

    DTUNITTEST_ASSERT_TRUE(rgba.r == 255);
    DTUNITTEST_ASSERT_TRUE(rgba.g == 255);
    DTUNITTEST_ASSERT_TRUE(rgba.b == 255);
    DTUNITTEST_ASSERT_TRUE(rgba.a == 0xFF);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_raw_to_rgba8888_red(void)
{
    dterr_t* dterr = NULL;
    dtrgb565_t c = dtrgb565_raw(0xF800);
    dtrgba8888_t rgba = dtrgb565_to_rgba8888(c);

    DTUNITTEST_ASSERT_TRUE(rgba.r == 255);
    DTUNITTEST_ASSERT_TRUE(rgba.g == 0);
    DTUNITTEST_ASSERT_TRUE(rgba.b == 0);
    DTUNITTEST_ASSERT_TRUE(rgba.a == 0xFF);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_raw_to_rgba8888_green(void)
{
    dterr_t* dterr = NULL;
    dtrgb565_t c = dtrgb565_raw(0x07E0);
    dtrgba8888_t rgba = dtrgb565_to_rgba8888(c);

    DTUNITTEST_ASSERT_TRUE(rgba.r == 0);
    DTUNITTEST_ASSERT_TRUE(rgba.g == 255);
    DTUNITTEST_ASSERT_TRUE(rgba.b == 0);
    DTUNITTEST_ASSERT_TRUE(rgba.a == 0xFF);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_raw_to_rgba8888_blue(void)
{
    dterr_t* dterr = NULL;
    dtrgb565_t c = dtrgb565_raw(0x001F);
    dtrgba8888_t rgba = dtrgb565_to_rgba8888(c);

    DTUNITTEST_ASSERT_TRUE(rgba.r == 0);
    DTUNITTEST_ASSERT_TRUE(rgba.g == 0);
    DTUNITTEST_ASSERT_TRUE(rgba.b == 255);
    DTUNITTEST_ASSERT_TRUE(rgba.a == 0xFF);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_component_accessors_primary_red(void)
{
    dterr_t* dterr = NULL;
    dtrgb565_t c = dtrgb565_raw(0xF800);

    DTUNITTEST_ASSERT_TRUE(dtrgb565_r(c) == 255);
    DTUNITTEST_ASSERT_TRUE(dtrgb565_g(c) == 0);
    DTUNITTEST_ASSERT_TRUE(dtrgb565_b(c) == 0);
    DTUNITTEST_ASSERT_TRUE(dtrgb565_a(c) == 0xFF);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_component_accessors_primary_green(void)
{
    dterr_t* dterr = NULL;
    dtrgb565_t c = dtrgb565_raw(0x07E0);

    DTUNITTEST_ASSERT_TRUE(dtrgb565_r(c) == 0);
    DTUNITTEST_ASSERT_TRUE(dtrgb565_g(c) == 255);
    DTUNITTEST_ASSERT_TRUE(dtrgb565_b(c) == 0);
    DTUNITTEST_ASSERT_TRUE(dtrgb565_a(c) == 0xFF);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_component_accessors_primary_blue(void)
{
    dterr_t* dterr = NULL;
    dtrgb565_t c = dtrgb565_raw(0x001F);

    DTUNITTEST_ASSERT_TRUE(dtrgb565_r(c) == 0);
    DTUNITTEST_ASSERT_TRUE(dtrgb565_g(c) == 0);
    DTUNITTEST_ASSERT_TRUE(dtrgb565_b(c) == 255);
    DTUNITTEST_ASSERT_TRUE(dtrgb565_a(c) == 0xFF);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_from_rgba8888_discards_alpha(void)
{
    dterr_t* dterr = NULL;
    dtrgba8888_t a = dtrgba8888(100, 150, 200, 0x00);
    dtrgba8888_t b = dtrgba8888(100, 150, 200, 0xFF);

    dtrgb565_t ca = dtrgb565_from_rgba8888(a);
    dtrgb565_t cb = dtrgb565_from_rgba8888(b);

    DTUNITTEST_ASSERT_TRUE(dtrgb565_compare(ca, cb) == 0);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_roundtrip_primary_red(void)
{
    dterr_t* dterr = NULL;
    dtrgba8888_t src = dtrgba8888(255, 0, 0, 0x7F);
    dtrgb565_t rgb565 = dtrgb565_from_rgba8888(src);
    dtrgba8888_t dst = dtrgb565_to_rgba8888(rgb565);

    DTUNITTEST_ASSERT_TRUE(rgb565.value == 0xF800);
    DTUNITTEST_ASSERT_TRUE(dst.r == 255);
    DTUNITTEST_ASSERT_TRUE(dst.g == 0);
    DTUNITTEST_ASSERT_TRUE(dst.b == 0);
    DTUNITTEST_ASSERT_TRUE(dst.a == 0xFF);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_roundtrip_primary_green(void)
{
    dterr_t* dterr = NULL;
    dtrgba8888_t src = dtrgba8888(0, 255, 0, 0x12);
    dtrgb565_t rgb565 = dtrgb565_from_rgba8888(src);
    dtrgba8888_t dst = dtrgb565_to_rgba8888(rgb565);

    DTUNITTEST_ASSERT_TRUE(rgb565.value == 0x07E0);
    DTUNITTEST_ASSERT_TRUE(dst.r == 0);
    DTUNITTEST_ASSERT_TRUE(dst.g == 255);
    DTUNITTEST_ASSERT_TRUE(dst.b == 0);
    DTUNITTEST_ASSERT_TRUE(dst.a == 0xFF);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_roundtrip_primary_blue(void)
{
    dterr_t* dterr = NULL;
    dtrgba8888_t src = dtrgba8888(0, 0, 255, 0x34);
    dtrgb565_t rgb565 = dtrgb565_from_rgba8888(src);
    dtrgba8888_t dst = dtrgb565_to_rgba8888(rgb565);

    DTUNITTEST_ASSERT_TRUE(rgb565.value == 0x001F);
    DTUNITTEST_ASSERT_TRUE(dst.r == 0);
    DTUNITTEST_ASSERT_TRUE(dst.g == 0);
    DTUNITTEST_ASSERT_TRUE(dst.b == 255);
    DTUNITTEST_ASSERT_TRUE(dst.a == 0xFF);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_compare_equal(void)
{
    dterr_t* dterr = NULL;
    dtrgb565_t a = dtrgb565_raw(0x1234);
    dtrgb565_t b = dtrgb565_raw(0x1234);

    DTUNITTEST_ASSERT_TRUE(dtrgb565_compare(a, b) == 0);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_compare_less_than(void)
{
    dterr_t* dterr = NULL;
    dtrgb565_t a = dtrgb565_raw(0x1234);
    dtrgb565_t b = dtrgb565_raw(0x1235);

    DTUNITTEST_ASSERT_TRUE(dtrgb565_compare(a, b) < 0);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_compare_greater_than(void)
{
    dterr_t* dterr = NULL;
    dtrgb565_t a = dtrgb565_raw(0x1235);
    dtrgb565_t b = dtrgb565_raw(0x1234);

    DTUNITTEST_ASSERT_TRUE(dtrgb565_compare(a, b) > 0);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_named_colors_primary_values(void)
{
    dterr_t* dterr = NULL;

    DTUNITTEST_ASSERT_TRUE(DTRGB565_BLACK.value == 0x0000);
    DTUNITTEST_ASSERT_TRUE(DTRGB565_WHITE.value == 0xFFFF);
    DTUNITTEST_ASSERT_TRUE(DTRGB565_RED.value == 0xF800);
    DTUNITTEST_ASSERT_TRUE(DTRGB565_LIME.value == 0x07E0);
    DTUNITTEST_ASSERT_TRUE(DTRGB565_BLUE.value == 0x001F);
    DTUNITTEST_ASSERT_TRUE(DTRGB565_YELLOW.value == 0xFFE0);
    DTUNITTEST_ASSERT_TRUE(DTRGB565_CYAN.value == 0x07FF);
    DTUNITTEST_ASSERT_TRUE(DTRGB565_MAGENTA.value == 0xF81F);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_named_color_inverse_to_rgba8888_red(void)
{
    dterr_t* dterr = NULL;
    dtrgba8888_t rgba = dtrgb565_to_rgba8888(DTRGB565_RED);

    DTUNITTEST_ASSERT_TRUE(dtrgba8888_compare(rgba, DTRGBA8888_RED) == 0);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_named_color_inverse_to_rgba8888_green(void)
{
    dterr_t* dterr = NULL;
    dtrgba8888_t rgba = dtrgb565_to_rgba8888(DTRGB565_GREEN);

    DTUNITTEST_ASSERT_TRUE(dtrgba8888_compare(rgba, DTRGBA8888_GREEN) == 0);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_named_color_inverse_to_rgba8888_blue(void)
{
    dterr_t* dterr = NULL;
    dtrgba8888_t rgba = dtrgb565_to_rgba8888(DTRGB565_BLUE);

    DTUNITTEST_ASSERT_TRUE(dtrgba8888_compare(rgba, DTRGBA8888_BLUE) == 0);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
test_dtrgb565_roundtrip_quantizes_mid_values(void)
{
    dterr_t* dterr = NULL;
    dtrgba8888_t src = dtrgba8888(123, 45, 67, 0xAA);

    dtrgb565_t rgb565 = dtrgb565_from_rgba8888(src);
    dtrgba8888_t dst = dtrgb565_to_rgba8888(rgb565);

    DTUNITTEST_ASSERT_TRUE(dst.a == 0xFF);
    DTUNITTEST_ASSERT_TRUE(dst.r != src.r || dst.g != src.g || dst.b != src.b);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
// suite runner
void
test_dtrgb565(DTUNITTEST_SUITE_ARGS)
{
    DTUNITTEST_RUN_TEST(test_dtrgb565_constructor_black);
    DTUNITTEST_RUN_TEST(test_dtrgb565_constructor_white);
    DTUNITTEST_RUN_TEST(test_dtrgb565_constructor_red);
    DTUNITTEST_RUN_TEST(test_dtrgb565_constructor_green);
    DTUNITTEST_RUN_TEST(test_dtrgb565_constructor_blue);

    DTUNITTEST_RUN_TEST(test_dtrgb565_raw_to_rgba8888_black);
    DTUNITTEST_RUN_TEST(test_dtrgb565_raw_to_rgba8888_white);
    DTUNITTEST_RUN_TEST(test_dtrgb565_raw_to_rgba8888_red);
    DTUNITTEST_RUN_TEST(test_dtrgb565_raw_to_rgba8888_green);
    DTUNITTEST_RUN_TEST(test_dtrgb565_raw_to_rgba8888_blue);

    DTUNITTEST_RUN_TEST(test_dtrgb565_component_accessors_primary_red);
    DTUNITTEST_RUN_TEST(test_dtrgb565_component_accessors_primary_green);
    DTUNITTEST_RUN_TEST(test_dtrgb565_component_accessors_primary_blue);

    DTUNITTEST_RUN_TEST(test_dtrgb565_from_rgba8888_discards_alpha);
    DTUNITTEST_RUN_TEST(test_dtrgb565_roundtrip_primary_red);
    DTUNITTEST_RUN_TEST(test_dtrgb565_roundtrip_primary_green);
    DTUNITTEST_RUN_TEST(test_dtrgb565_roundtrip_primary_blue);
    DTUNITTEST_RUN_TEST(test_dtrgb565_roundtrip_quantizes_mid_values);

    DTUNITTEST_RUN_TEST(test_dtrgb565_compare_equal);
    DTUNITTEST_RUN_TEST(test_dtrgb565_compare_less_than);
    DTUNITTEST_RUN_TEST(test_dtrgb565_compare_greater_than);

    DTUNITTEST_RUN_TEST(test_dtrgb565_named_colors_primary_values);
    DTUNITTEST_RUN_TEST(test_dtrgb565_named_color_inverse_to_rgba8888_red);
    DTUNITTEST_RUN_TEST(test_dtrgb565_named_color_inverse_to_rgba8888_green);
    DTUNITTEST_RUN_TEST(test_dtrgb565_named_color_inverse_to_rgba8888_blue);
}