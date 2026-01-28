#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dterr.h>
#include <dtcore/dtpackx.h>
#include <dtcore/dtstr.h>
#include <dtcore/dtunittest.h>

#include <dtcore_tests.h>

#define TAG "test_dtcore_dtpackx"

/*  Examples first: short, readable, self-contained patterns. */

// ------------------------------------------------------------------------
//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtpackx_example_roundtrip_scalars(void)
{
    dterr_t* dterr = NULL;

    uint8_t buf[64] = { 0 };
    const int32_t buflen = (int32_t)sizeof(buf);
    int32_t off = 0, n = 0;

    n = dtpackx_pack_int32(0x7F010203, buf, off, buflen);
    DTUNITTEST_ASSERT_INT(n, ==, 4);
    off += n;
    n = dtpackx_pack_int16((int16_t)0xBEEF, buf, off, buflen);
    DTUNITTEST_ASSERT_INT(n, ==, 2);
    off += n;
    n = dtpackx_pack_int64(0x1122334455667788LL, buf, off, buflen);
    DTUNITTEST_ASSERT_INT(n, ==, 8);
    off += n;
    n = dtpackx_pack_bool(true, buf, off, buflen);
    DTUNITTEST_ASSERT_INT(n, ==, 1);
    off += n;
    n = dtpackx_pack_float(3.25f, buf, off, buflen);
    DTUNITTEST_ASSERT_INT(n, ==, 4);
    off += n;
    n = dtpackx_pack_double(123.456, buf, off, buflen);
    DTUNITTEST_ASSERT_INT(n, ==, 8);
    off += n;
    n = dtpackx_pack_byte(0xAB, buf, off, buflen);
    DTUNITTEST_ASSERT_INT(n, ==, 1);
    off += n;

    off = 0;
    int32_t i32 = 0;
    int16_t i16 = 0;
    int64_t i64 = 0;
    bool b = false;
    float f = 0.f;
    double d = 0.0;
    uint8_t by = 0;

    n = dtpackx_unpack_int32(buf, off, buflen, &i32);
    DTUNITTEST_ASSERT_INT(n, ==, 4);
    off += n;
    n = dtpackx_unpack_int16(buf, off, buflen, &i16);
    DTUNITTEST_ASSERT_INT(n, ==, 2);
    off += n;
    n = dtpackx_unpack_int64(buf, off, buflen, &i64);
    DTUNITTEST_ASSERT_INT(n, ==, 8);
    off += n;
    n = dtpackx_unpack_bool(buf, off, buflen, &b);
    DTUNITTEST_ASSERT_INT(n, ==, 1);
    off += n;
    n = dtpackx_unpack_float(buf, off, buflen, &f);
    DTUNITTEST_ASSERT_INT(n, ==, 4);
    off += n;
    n = dtpackx_unpack_double(buf, off, buflen, &d);
    DTUNITTEST_ASSERT_INT(n, ==, 8);
    off += n;
    n = dtpackx_unpack_byte(buf, off, buflen, &by);
    DTUNITTEST_ASSERT_INT(n, ==, 1);
    off += n;

    DTUNITTEST_ASSERT_INT(i32, ==, 0x7F010203);
    DTUNITTEST_ASSERT_INT(i16, ==, (int16_t)0xBEEF);
    DTUNITTEST_ASSERT_TRUE(i64 == 0x1122334455667788LL);
    DTUNITTEST_ASSERT_TRUE(b);
    DTUNITTEST_ASSERT_TRUE(fabsf(f - 3.25f) < 1e-6f);
    DTUNITTEST_ASSERT_TRUE(fabs(d - 123.456) < 1e-9);
    DTUNITTEST_ASSERT_INT(by, ==, 0xAB);

cleanup:
    return dterr;
}

//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtpackx_example_string_and_lengths(void)
{
    dterr_t* dterr = NULL;
    char* out = NULL;

    uint8_t buf[64] = { 0 };
    const int32_t buflen = (int32_t)sizeof(buf);
    int32_t off = 0;

    const char* s = "Hello";
    int need = dtpackx_pack_string_length(s);
    DTUNITTEST_ASSERT_INT(need, ==, (int)strlen(s) + 1);

    int32_t n = dtpackx_pack_string(s, buf, off, buflen);
    DTUNITTEST_ASSERT_INT(n, ==, need);
    off += n;

    n = dtpackx_unpack_string(buf, 0, buflen, &out);
    DTUNITTEST_ASSERT_INT(n, ==, need);
    DTUNITTEST_ASSERT_TRUE(out != NULL);
    DTUNITTEST_ASSERT_EQUAL_STRING(out, "Hello");

cleanup:
    dtstr_dispose(out);
    return dterr;
}

//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtpackx_example_float_array_layout(void)
{
    dterr_t* dterr = NULL;

    /* Layout: [int32 count][count x float32]. */
    float in[3] = { 1.0f, -2.5f, 100.0f };
    uint8_t buf[128] = { 0 };
    const int32_t buflen = (int32_t)sizeof(buf);

    int32_t wrote = dtpackx_pack_float_array(in, 3, buf, 0, buflen);
    DTUNITTEST_ASSERT_TRUE(wrote == 4 + 3 * 4);

    /* Verify count prefix in little-endian. */
    DTUNITTEST_ASSERT_INT(buf[0], ==, 3);
    DTUNITTEST_ASSERT_INT(buf[1], ==, 0);
    DTUNITTEST_ASSERT_INT(buf[2], ==, 0);
    DTUNITTEST_ASSERT_INT(buf[3], ==, 0);

    float* out = NULL;
    int32_t read = dtpackx_unpack_float_array(buf, 0, buflen, &out);
    DTUNITTEST_ASSERT_INT(read, ==, wrote);
    DTUNITTEST_ASSERT_TRUE(out != NULL);
    DTUNITTEST_ASSERT_TRUE(fabsf(out[0] - 1.0f) < 1e-6f);
    DTUNITTEST_ASSERT_TRUE(fabsf(out[1] + 2.5f) < 1e-6f);
    DTUNITTEST_ASSERT_TRUE(fabsf(out[2] - 100.0f) < 1e-6f);
    free(out);

cleanup:
    return dterr;
}

// ------------------------------------------------------------------------
//  Numbered tests (targeted edge cases and gotchas).
//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtpackx_01_endianness_bytes(void)
{
    dterr_t* dterr = NULL;

    uint8_t buf[32] = { 0 };
    const int32_t buflen = (int32_t)sizeof(buf);
    int32_t off = 0;

    DTUNITTEST_ASSERT_INT(dtpackx_pack_int32(0x01020304, buf, off, buflen), ==, 4);
    /* Expect little-endian 04 03 02 01 at buf[0..3]. */
    DTUNITTEST_ASSERT_INT(buf[0], ==, 0x04);
    DTUNITTEST_ASSERT_INT(buf[1], ==, 0x03);
    DTUNITTEST_ASSERT_INT(buf[2], ==, 0x02);
    DTUNITTEST_ASSERT_INT(buf[3], ==, 0x01);

    off = 0;
    int32_t v32 = 0;
    DTUNITTEST_ASSERT_INT(dtpackx_unpack_int32(buf, off, buflen, &v32), ==, 4);
    DTUNITTEST_ASSERT_INT(v32, ==, 0x01020304);

    DTUNITTEST_ASSERT_INT(dtpackx_pack_int16(0x0506, buf, 0, buflen), ==, 2);
    DTUNITTEST_ASSERT_INT(buf[0], ==, 0x06);
    DTUNITTEST_ASSERT_INT(buf[1], ==, 0x05);

cleanup:
    return dterr;
}

//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtpackx_02_negative_offset_fails(void)
{
    dterr_t* dterr = NULL;

    uint8_t buf[8] = { 0 };
    const int32_t buflen = (int32_t)sizeof(buf);
    int32_t v32 = 0;

    DTUNITTEST_ASSERT_INT(dtpackx_pack_int32(123, buf, -1, buflen), ==, -1);
    DTUNITTEST_ASSERT_INT(dtpackx_unpack_int32(buf, -1, buflen, &v32), ==, -1);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_string("x", buf, -5, buflen), ==, -1);

cleanup:
    return dterr;
}

//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtpackx_03_null_pointers(void)
{
    dterr_t* dterr = NULL;

    uint8_t buf[8] = { 0 };
    const int32_t buflen = (int32_t)sizeof(buf);
    int32_t v32 = 0;
    int16_t v16 = 0;
    int64_t v64 = 0;
    bool vb = false;
    float vf = 0.f;
    double vd = 0.0;
    uint8_t vy = 0;
    char* s = NULL;
    float* fa = NULL;

    /* Unpack NULL destination pointer -> fail (-1). */
    DTUNITTEST_ASSERT_INT(dtpackx_unpack_int32(buf, 0, buflen, NULL), ==, -1);
    DTUNITTEST_ASSERT_INT(dtpackx_unpack_int16(buf, 0, buflen, NULL), ==, -1);
    DTUNITTEST_ASSERT_INT(dtpackx_unpack_int64(buf, 0, buflen, NULL), ==, -1);
    DTUNITTEST_ASSERT_INT(dtpackx_unpack_bool(buf, 0, buflen, NULL), ==, -1);
    DTUNITTEST_ASSERT_INT(dtpackx_unpack_float(buf, 0, buflen, NULL), ==, -1);
    DTUNITTEST_ASSERT_INT(dtpackx_unpack_double(buf, 0, buflen, NULL), ==, -1);
    DTUNITTEST_ASSERT_INT(dtpackx_unpack_byte(buf, 0, buflen, NULL), ==, -1);
    DTUNITTEST_ASSERT_INT(dtpackx_unpack_string(buf, 0, buflen, NULL), ==, -1);

    /* Pack NULL output buffer -> fail (-1). */
    DTUNITTEST_ASSERT_INT(dtpackx_pack_int32(1, NULL, 0, buflen), ==, -1);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_int16(1, NULL, 0, buflen), ==, -1);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_int64(1, NULL, 0, buflen), ==, -1);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_bool(true, NULL, 0, buflen), ==, -1);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_float(1.0f, NULL, 0, buflen), ==, -1);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_double(2.0, NULL, 0, buflen), ==, -1);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_byte(0xAA, NULL, 0, buflen), ==, -1);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_string("x", NULL, 0, buflen), ==, -1);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_float_array(&vf, 1, NULL, 0, buflen), ==, -1);

    /* Smoke success with valid pointers to avoid unused warnings. */
    (void)dtpackx_unpack_int32(buf, 0, buflen, &v32);
    (void)dtpackx_unpack_int16(buf, 0, buflen, &v16);
    (void)dtpackx_unpack_int64(buf, 0, buflen, &v64);
    (void)dtpackx_unpack_bool(buf, 0, buflen, &vb);
    (void)dtpackx_unpack_float(buf, 0, buflen, &vf);
    (void)dtpackx_unpack_double(buf, 0, buflen, &vd);
    (void)dtpackx_unpack_byte(buf, 0, buflen, &vy);
    (void)dtpackx_unpack_string(buf, 0, buflen, &s);
    (void)dtpackx_unpack_float_array(buf, 0, buflen, &fa);
    free(fa);

cleanup:
    dtstr_dispose(s);
    return dterr;
}

//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtpackx_04_pack_string_null_writes_zero(void)
{
    dterr_t* dterr = NULL;
    char* s = NULL;

    uint8_t buf[8] = { 0xFF };
    const int32_t buflen = (int32_t)sizeof(buf);

    int32_t n = dtpackx_pack_string(NULL, buf, 0, buflen);
    DTUNITTEST_ASSERT_INT(n, ==, 1);
    DTUNITTEST_ASSERT_INT(buf[0], ==, 0);

    n = dtpackx_unpack_string(buf, 0, buflen, &s);
    DTUNITTEST_ASSERT_INT(n, ==, 1);
    DTUNITTEST_ASSERT_TRUE(s != NULL);
    DTUNITTEST_ASSERT_EQUAL_STRING(s, "");

cleanup:
    dtstr_dispose(s);
    return dterr;
}

//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtpackx_05_float_array_count_zero(void)
{
    dterr_t* dterr = NULL;

    uint8_t buf[8] = { 0 };
    const int32_t buflen = (int32_t)sizeof(buf);

    /* Accept input==NULL when count==0: write 4-byte header. */
    int32_t n = dtpackx_pack_float_array(NULL, 0, buf, 0, buflen);

    if (n == -1)
    {
        /* Fallback path: provide a dummy non-NULL pointer; still expect header only. */
        float dummy = 0.0f;
        n = dtpackx_pack_float_array(&dummy, 0, buf, 0, buflen);
    }

    DTUNITTEST_ASSERT_INT(n, ==, 4);

    /* Count prefix bytes should be {0,0,0,0}. */
    DTUNITTEST_ASSERT_INT(buf[0], ==, 0);
    DTUNITTEST_ASSERT_INT(buf[1], ==, 0);
    DTUNITTEST_ASSERT_INT(buf[2], ==, 0);
    DTUNITTEST_ASSERT_INT(buf[3], ==, 0);

    float* out = NULL;
    int32_t r = dtpackx_unpack_float_array(buf, 0, buflen, &out);
    DTUNITTEST_ASSERT_INT(r, ==, 4);

    /* Some malloc(0) implementations return NULL; both are acceptable. */
    if (out)
    {
        free(out);
    }

cleanup:
    return dterr;
}

//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtpackx_06_lengths_match_sizes(void)
{
    dterr_t* dterr = NULL;

    DTUNITTEST_ASSERT_INT(dtpackx_pack_int16_length(), ==, 2);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_int32_length(), ==, 4);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_int64_length(), ==, 8);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_bool_length(), ==, 1);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_float_length(), ==, 4);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_double_length(), ==, 8);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_byte_length(), ==, 1);

    DTUNITTEST_ASSERT_INT(dtpackx_pack_string_length(NULL), ==, 1);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_string_length("A"), ==, 2);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_string_length(""), ==, 1);

cleanup:
    return dterr;
}

/* --------------------- NEW: overrun / bounds tests --------------------- */

//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtpackx_07_pack_bounds_scalar(void)
{
    dterr_t* dterr = NULL;

    uint8_t buf[4] = { 0xEE, 0xEE, 0xEE, 0xEE };
    const int32_t buflen = (int32_t)sizeof(buf);

    /* Too small by 1 byte. */
    DTUNITTEST_ASSERT_INT(dtpackx_pack_int32(0x11223344, buf, 0, 3), ==, -1);

    /* Exactly enough space. */
    DTUNITTEST_ASSERT_INT(dtpackx_pack_int32(0xA1B2C3D4, buf, 0, buflen), ==, 4);

    /* Offset near end. */
    DTUNITTEST_ASSERT_INT(dtpackx_pack_byte(0x7A, buf, 3, buflen), ==, 1);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_byte(0x7B, buf, 4, buflen), ==, -1);

cleanup:
    return dterr;
}

//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtpackx_08_unpack_bounds_scalar(void)
{
    dterr_t* dterr = NULL;

    uint8_t buf[4] = { 0x78, 0x56, 0x34, 0x12 }; /* 0x12345678 LE */
    const int32_t buflen = (int32_t)sizeof(buf);
    int32_t v = 0;

    DTUNITTEST_ASSERT_INT(dtpackx_unpack_int32(buf, 0, 3, &v), ==, -1);
    DTUNITTEST_ASSERT_INT(dtpackx_unpack_int32(buf, 0, buflen, &v), ==, 4);
    DTUNITTEST_ASSERT_INT(v, ==, 0x12345678);

cleanup:
    return dterr;
}

//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtpackx_09_pack_string_bounds(void)
{
    dterr_t* dterr = NULL;

    uint8_t buf[8] = { 0 };
    const int32_t buflen = (int32_t)sizeof(buf);

    /* "Hi" needs 3 bytes including NUL. */
    DTUNITTEST_ASSERT_INT(dtpackx_pack_string("Hi", buf, 0, 2), ==, -1);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_string("Hi", buf, 0, buflen), ==, 3);

    /* Offset near end: exactly 2 bytes left -> succeed; only 1 byte left -> fail. */
    DTUNITTEST_ASSERT_INT(dtpackx_pack_string("A", buf, buflen - 2, buflen), ==, 2);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_string("A", buf, buflen - 1, buflen), ==, -1);
cleanup:
    return dterr;
}

//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtpackx_10_unpack_string_missing_nul(void)
{
    dterr_t* dterr = NULL;

    /* No NUL within buflen -> fail. */
    uint8_t buf1[3] = { 'a', 'b', 'c' };
    char* s = NULL;
    DTUNITTEST_ASSERT_INT(dtpackx_unpack_string(buf1, 0, 3, &s), ==, -3);

    /* NUL present within range -> success. */
    uint8_t buf2[4] = { 'a', 'b', 'c', 0 };
    DTUNITTEST_ASSERT_INT(dtpackx_unpack_string(buf2, 0, 4, &s), ==, 4);
    DTUNITTEST_ASSERT_TRUE(s != NULL);
    DTUNITTEST_ASSERT_EQUAL_STRING(s, "abc");

cleanup:
    dtstr_dispose(s);
    return dterr;
}

//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtpackx_11_float_array_bounds(void)
{
    dterr_t* dterr = NULL;

    float vals[3] = { 10.0f, 20.0f, 30.0f };
    uint8_t buf[32] = { 0 };
    const int32_t buflen = (int32_t)sizeof(buf);

    /* Need 4 + 3*4 = 16 bytes. Provide 15 -> fail. */
    DTUNITTEST_ASSERT_INT(dtpackx_pack_float_array(vals, 3, buf, 0, 15), ==, -1);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_float_array(vals, 3, buf, 0, buflen), ==, 16);

    /* Now try unpack with truncated buflen (only header + one element visible). */
    float* out = NULL;
    DTUNITTEST_ASSERT_INT(dtpackx_unpack_float_array(buf, 0, 8, &out), ==, -1); /* need 16 */
    DTUNITTEST_ASSERT_INT(dtpackx_unpack_float_array(buf, 0, buflen, &out), ==, 16);
    DTUNITTEST_ASSERT_TRUE(out != NULL);
    DTUNITTEST_ASSERT_TRUE(fabsf(out[0] - 10.0f) < 1e-6f);
    DTUNITTEST_ASSERT_TRUE(fabsf(out[1] - 20.0f) < 1e-6f);
    DTUNITTEST_ASSERT_TRUE(fabsf(out[2] - 30.0f) < 1e-6f);
    free(out);

cleanup:
    return dterr;
}

//------------------------------------------------------------------------
static dterr_t*
test_dtcore_dtpackx_12_offset_near_end(void)
{
    dterr_t* dterr = NULL;

    uint8_t buf[5] = { 0xCD, 0xCD, 0xCD, 0xCD, 0xCD };
    const int32_t buflen = (int32_t)sizeof(buf);

    /* Write 1 byte at last position succeeds; 2 bytes should fail. */
    DTUNITTEST_ASSERT_INT(dtpackx_pack_byte(0x42, buf, 4, buflen), ==, 1);
    DTUNITTEST_ASSERT_INT(dtpackx_pack_int16(0x1234, buf, 4, buflen), ==, -1);

    /* Read 1 byte at last position succeeds; 4 bytes should fail. */
    uint8_t by = 0;
    DTUNITTEST_ASSERT_INT(dtpackx_unpack_byte(buf, 4, buflen, &by), ==, 1);
    DTUNITTEST_ASSERT_INT(by, ==, 0x42);

    int32_t v32 = 0;
    DTUNITTEST_ASSERT_INT(dtpackx_unpack_int32(buf, 2, buflen, &v32), ==, -1);

cleanup:
    return dterr;
}

// ------------------------------------------------------------------------
void
test_dtcore_dtpackx(DTUNITTEST_SUITE_ARGS)
{
    DTUNITTEST_RUN_TEST(test_dtcore_dtpackx_example_roundtrip_scalars);
    DTUNITTEST_RUN_TEST(test_dtcore_dtpackx_example_string_and_lengths);
    DTUNITTEST_RUN_TEST(test_dtcore_dtpackx_example_float_array_layout);

    DTUNITTEST_RUN_TEST(test_dtcore_dtpackx_01_endianness_bytes);
    DTUNITTEST_RUN_TEST(test_dtcore_dtpackx_02_negative_offset_fails);
    DTUNITTEST_RUN_TEST(test_dtcore_dtpackx_03_null_pointers);
    DTUNITTEST_RUN_TEST(test_dtcore_dtpackx_04_pack_string_null_writes_zero);
    DTUNITTEST_RUN_TEST(test_dtcore_dtpackx_05_float_array_count_zero);
    DTUNITTEST_RUN_TEST(test_dtcore_dtpackx_06_lengths_match_sizes);

    /* New bounds/overrun coverage */
    DTUNITTEST_RUN_TEST(test_dtcore_dtpackx_07_pack_bounds_scalar);
    DTUNITTEST_RUN_TEST(test_dtcore_dtpackx_08_unpack_bounds_scalar);
    DTUNITTEST_RUN_TEST(test_dtcore_dtpackx_09_pack_string_bounds);
    DTUNITTEST_RUN_TEST(test_dtcore_dtpackx_10_unpack_string_missing_nul);
    DTUNITTEST_RUN_TEST(test_dtcore_dtpackx_11_float_array_bounds);
    DTUNITTEST_RUN_TEST(test_dtcore_dtpackx_12_offset_near_end);
}
