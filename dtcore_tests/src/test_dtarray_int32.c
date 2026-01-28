#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dtarray_int32.h>
#include <dtcore/dterr.h>
#include <dtcore/dtunittest.h>
#include <dtcore_tests.h>

// Example: smallest useful roundtrip using 3 values.
static dterr_t*
test_dtcore_dtarray_int32_example_roundtrip(void)
{
    dterr_t* dterr = NULL;
    dtarray_int32_t* a = NULL;
    dtarray_int32_t* b = NULL;
    uint8_t* buf = NULL;

    DTUNITTEST_ASSERT_NULL(dtarray_int32_create(&a, 3));
    a->items[0] = 10;
    a->items[1] = -20;
    a->items[2] = 30;

    const int32_t need = dtarray_int32_pack_length(a);
    buf = (uint8_t*)calloc((size_t)need, 1);
    DTUNITTEST_ASSERT_NOT_NULL(buf);

    int32_t wrote = dtarray_int32_pack(a, buf, 0, need);
    DTUNITTEST_ASSERT_INT(wrote, ==, need);

    DTUNITTEST_ASSERT_NULL(dtarray_int32_create(&b, 3));
    int32_t read = dtarray_int32_unpack(b, buf, 0, need);
    DTUNITTEST_ASSERT_INT(read, ==, need);

    DTUNITTEST_ASSERT_INT(b->items[0], ==, 10);
    DTUNITTEST_ASSERT_INT(b->items[1], ==, -20);
    DTUNITTEST_ASSERT_INT(b->items[2], ==, 30);

cleanup:
    dtarray_int32_dispose(b);
    dtarray_int32_dispose(a);
    free(buf);
    return dterr;
}
// ----------------------------------------------------------------------------------------------------------------------------------------

static dterr_t*
test_dtcore_dtarray_int32_example_truncate_to_fit(void)
{
    dterr_t* dterr = NULL;
    dtarray_int32_t* src = NULL;
    dtarray_int32_t* dst = NULL;
    uint8_t* buf = NULL;

    DTUNITTEST_ASSERT_NULL(dtarray_int32_create(&src, 3));
    src->items[0] = 111;
    src->items[1] = 222;
    src->items[2] = 333;

    const int32_t need = dtarray_int32_pack_length(src);
    buf = (uint8_t*)malloc((size_t)need);
    DTUNITTEST_ASSERT_NOT_NULL(buf);
    DTUNITTEST_ASSERT_INT(dtarray_int32_pack(src, buf, 0, need), ==, need);

    DTUNITTEST_ASSERT_NULL(dtarray_int32_create(&dst, 2));
    DTUNITTEST_ASSERT_INT(dtarray_int32_unpack(dst, buf, 0, need), ==, need);
    DTUNITTEST_ASSERT_INT(dst->items[0], ==, 111);
    DTUNITTEST_ASSERT_INT(dst->items[1], ==, 222);

cleanup:
    dtarray_int32_dispose(dst);
    dtarray_int32_dispose(src);
    free(buf);
    return dterr;
}
// ----------------------------------------------------------------------------------------------------------------------------------------

static dterr_t*
test_dtcore_dtarray_int32_example_pack_at_offset(void)
{
    dterr_t* dterr = NULL;
    dtarray_int32_t* a = NULL;
    uint8_t* buf = NULL;

    DTUNITTEST_ASSERT_NULL(dtarray_int32_create(&a, 2));
    a->items[0] = 7;
    a->items[1] = -8;

    const int32_t need = dtarray_int32_pack_length(a);
    const int32_t offset = 5;
    buf = (uint8_t*)calloc((size_t)(offset + need + 3), 1);
    DTUNITTEST_ASSERT_NOT_NULL(buf);

    DTUNITTEST_ASSERT_INT(dtarray_int32_pack(a, buf, offset, offset + need), ==, need);

    const uint8_t expected[12] = { 2, 0, 0, 0, 7, 0, 0, 0, 0xF8, 0xFF, 0xFF, 0xFF };
    DTUNITTEST_ASSERT_EQUAL_BYTES(buf + offset, expected, sizeof(expected));

cleanup:
    dtarray_int32_dispose(a);
    free(buf);
    return dterr;
}
// ----------------------------------------------------------------------------------------------------------------------------------------

static dterr_t*
test_dtcore_dtarray_int32_01_create_bad_args(void)
{
    dterr_t* dterr = NULL;
    dtarray_int32_t* a = NULL;

    dterr = dtarray_int32_create(NULL, 3);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_ARGUMENT_NULL);

    dterr = dtarray_int32_create(&a, 0);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_BADARG);

    DTUNITTEST_ASSERT_NULL(a);

cleanup:
    dtarray_int32_dispose(a);
    return dterr;
}
// ----------------------------------------------------------------------------------------------------------------------------------------

static dterr_t*
test_dtcore_dtarray_int32_02_create_ok_and_dispose(void)
{
    dterr_t* dterr = NULL;
    dtarray_int32_t* a = NULL;

    DTUNITTEST_ASSERT_NULL(dtarray_int32_create(&a, 3));
    DTUNITTEST_ASSERT_NOT_NULL(a);
    DTUNITTEST_ASSERT_INT(a->count, ==, 3);
    DTUNITTEST_ASSERT_NOT_NULL(a->items);
    DTUNITTEST_ASSERT_INT(a->items[0], ==, 0);
    DTUNITTEST_ASSERT_INT(a->items[1], ==, 0);
    DTUNITTEST_ASSERT_INT(a->items[2], ==, 0);

cleanup:
    dtarray_int32_dispose(a);
    return dterr;
}
// ----------------------------------------------------------------------------------------------------------------------------------------

static dterr_t*
test_dtcore_dtarray_int32_03_pack_length_math(void)
{
    dterr_t* dterr = NULL;
    dtarray_int32_t* a = NULL;

    DTUNITTEST_ASSERT_NULL(dtarray_int32_create(&a, 5));
    const int32_t L = dtarray_int32_pack_length(a);
    DTUNITTEST_ASSERT_INT(L, ==, (1 + 5) * 4);

cleanup:
    dtarray_int32_dispose(a);
    return dterr;
}
// ----------------------------------------------------------------------------------------------------------------------------------------

static dterr_t*
test_dtcore_dtarray_int32_04_pack_insufficient_buffer(void)
{
    dterr_t* dterr = NULL;
    dtarray_int32_t* a = NULL;
    uint8_t* buf = NULL;

    DTUNITTEST_ASSERT_NULL(dtarray_int32_create(&a, 2));
    a->items[0] = 1;
    a->items[1] = 2;

    const int32_t need = dtarray_int32_pack_length(a);
    buf = (uint8_t*)calloc((size_t)need, 1);
    DTUNITTEST_ASSERT_NOT_NULL(buf);

    DTUNITTEST_ASSERT_INT(dtarray_int32_pack(a, buf, 0, need - 1), ==, -1);

    const int32_t off = 3;
    memset(buf, 0, (size_t)need);
    DTUNITTEST_ASSERT_INT(dtarray_int32_pack(a, buf, off, off + need - 1), ==, -1);

cleanup:
    dtarray_int32_dispose(a);
    free(buf);
    return dterr;
}
// ----------------------------------------------------------------------------------------------------------------------------------------

static dterr_t*
test_dtcore_dtarray_int32_05_unpack_insufficient_buffer(void)
{
    dterr_t* dterr = NULL;
    dtarray_int32_t* src = NULL;
    dtarray_int32_t* dst = NULL;
    uint8_t* buf = NULL;

    DTUNITTEST_ASSERT_NULL(dtarray_int32_create(&src, 3));
    src->items[0] = 11;
    src->items[1] = 22;
    src->items[2] = 33;

    const int32_t need = dtarray_int32_pack_length(src);
    buf = (uint8_t*)malloc((size_t)need);
    DTUNITTEST_ASSERT_NOT_NULL(buf);
    DTUNITTEST_ASSERT_INT(dtarray_int32_pack(src, buf, 0, need), ==, need);

    DTUNITTEST_ASSERT_NULL(dtarray_int32_create(&dst, 3));
    DTUNITTEST_ASSERT_INT(dtarray_int32_unpack(dst, buf, 0, need - 1), ==, -1);

cleanup:
    dtarray_int32_dispose(dst);
    dtarray_int32_dispose(src);
    free(buf);
    return dterr;
}
// ----------------------------------------------------------------------------------------------------------------------------------------

static dterr_t*
test_dtcore_dtarray_int32_06_unpack_shorter_payload_tail_zeroed(void)
{
    dterr_t* dterr = NULL;
    dtarray_int32_t* dst = NULL;
    dtarray_int32_t* src = NULL;
    uint8_t* buf = NULL;

    DTUNITTEST_ASSERT_NULL(dtarray_int32_create(&dst, 4));
    dst->items[0] = 9;
    dst->items[1] = 9;
    dst->items[2] = 9;
    dst->items[3] = 9;

    DTUNITTEST_ASSERT_NULL(dtarray_int32_create(&src, 2));
    src->items[0] = 1;
    src->items[1] = 2;

    const int32_t need = dtarray_int32_pack_length(src);
    buf = (uint8_t*)malloc((size_t)need);
    DTUNITTEST_ASSERT_NOT_NULL(buf);
    DTUNITTEST_ASSERT_INT(dtarray_int32_pack(src, buf, 0, need), ==, need);

    DTUNITTEST_ASSERT_INT(dtarray_int32_unpack(dst, buf, 0, need), ==, need);
    DTUNITTEST_ASSERT_INT(dst->items[0], ==, 1);
    DTUNITTEST_ASSERT_INT(dst->items[1], ==, 2);
    DTUNITTEST_ASSERT_INT(dst->items[2], ==, 0);
    DTUNITTEST_ASSERT_INT(dst->items[3], ==, 0);

cleanup:
    dtarray_int32_dispose(src);
    dtarray_int32_dispose(dst);
    free(buf);
    return dterr;
}
// ----------------------------------------------------------------------------------------------------------------------------------------

static dterr_t*
test_dtcore_dtarray_int32_07_unpack_longer_payload_discard_extra(void)
{
    dterr_t* dterr = NULL;
    dtarray_int32_t* src = NULL;
    dtarray_int32_t* dst2 = NULL;
    uint8_t* buf = NULL;

    DTUNITTEST_ASSERT_NULL(dtarray_int32_create(&src, 3));
    src->items[0] = 111;
    src->items[1] = 222;
    src->items[2] = 333;

    const int32_t need = dtarray_int32_pack_length(src);
    buf = (uint8_t*)malloc((size_t)need);
    DTUNITTEST_ASSERT_NOT_NULL(buf);
    DTUNITTEST_ASSERT_INT(dtarray_int32_pack(src, buf, 0, need), ==, need);

    DTUNITTEST_ASSERT_NULL(dtarray_int32_create(&dst2, 2));
    DTUNITTEST_ASSERT_INT(dtarray_int32_unpack(dst2, buf, 0, need), ==, need);
    DTUNITTEST_ASSERT_INT(dst2->items[0], ==, 111);
    DTUNITTEST_ASSERT_INT(dst2->items[1], ==, 222);

cleanup:
    dtarray_int32_dispose(dst2);
    dtarray_int32_dispose(src);
    free(buf);
    return dterr;
}
// ----------------------------------------------------------------------------------------------------------------------------------------

static dterr_t*
test_dtcore_dtarray_int32_08_pack_with_nonzero_offset_and_exact_bytes(void)
{
    dterr_t* dterr = NULL;
    dtarray_int32_t* a = NULL;
    uint8_t* buf = NULL;

    DTUNITTEST_ASSERT_NULL(dtarray_int32_create(&a, 2));
    a->items[0] = 7;
    a->items[1] = -8;

    const int32_t need = dtarray_int32_pack_length(a);
    const int32_t offset = 5;
    buf = (uint8_t*)calloc((size_t)(offset + need + 1), 1);
    DTUNITTEST_ASSERT_NOT_NULL(buf);

    int32_t wrote = dtarray_int32_pack(a, buf, offset, offset + need);
    DTUNITTEST_ASSERT_INT(wrote, ==, need);

    const uint8_t expected[12] = { 2, 0, 0, 0, 7, 0, 0, 0, 0xF8, 0xFF, 0xFF, 0xFF };
    DTUNITTEST_ASSERT_EQUAL_BYTES(buf + offset, expected, sizeof(expected));

cleanup:
    dtarray_int32_dispose(a);
    free(buf);
    return dterr;
}
// ----------------------------------------------------------------------------------------------------------------------------------------

void
test_dtcore_dtarray_int32(DTUNITTEST_SUITE_ARGS)
{
    DTUNITTEST_RUN_TEST(test_dtcore_dtarray_int32_example_roundtrip);
    DTUNITTEST_RUN_TEST(test_dtcore_dtarray_int32_example_truncate_to_fit);
    DTUNITTEST_RUN_TEST(test_dtcore_dtarray_int32_example_pack_at_offset);

    DTUNITTEST_RUN_TEST(test_dtcore_dtarray_int32_01_create_bad_args);
    DTUNITTEST_RUN_TEST(test_dtcore_dtarray_int32_02_create_ok_and_dispose);
    DTUNITTEST_RUN_TEST(test_dtcore_dtarray_int32_03_pack_length_math);
    DTUNITTEST_RUN_TEST(test_dtcore_dtarray_int32_04_pack_insufficient_buffer);
    DTUNITTEST_RUN_TEST(test_dtcore_dtarray_int32_05_unpack_insufficient_buffer);
    DTUNITTEST_RUN_TEST(test_dtcore_dtarray_int32_06_unpack_shorter_payload_tail_zeroed);
    DTUNITTEST_RUN_TEST(test_dtcore_dtarray_int32_07_unpack_longer_payload_discard_extra);
    DTUNITTEST_RUN_TEST(test_dtcore_dtarray_int32_08_pack_with_nonzero_offset_and_exact_bytes);
}
