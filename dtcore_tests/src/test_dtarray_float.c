// test_dtarray_float.c
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dtarray_float.h>
#include <dtcore/dterr.h>
#include <dtcore/dtunittest.h>

// -----------------------------------------------------------------------------
// Tests

static dterr_t*
test_dtarray_float_create_bad_args(void)
{
    dterr_t* dterr = NULL;

    dtarray_float_t* a = NULL;
    dterr = dtarray_float_create(NULL, 3);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_ARGUMENT_NULL);

    dterr = dtarray_float_create(&a, 0); // count <= 0 is invalid per impl
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_BADARG);

    // Ensure no allocation snuck through
    DTUNITTEST_ASSERT_NULL(a);

cleanup:
    if (a)
        dtarray_float_dispose(a);
    return dterr;
}

static dterr_t*
test_dtarray_float_create_ok_and_dispose(void)
{
    dterr_t* dterr = NULL;

    dtarray_float_t* a = NULL;
    dterr = dtarray_float_create(&a, 3);
    DTUNITTEST_ASSERT_NULL(dterr);
    DTUNITTEST_ASSERT_NOT_NULL(a);
    DTUNITTEST_ASSERT_INT(a->count, ==, 3);
    DTUNITTEST_ASSERT_NOT_NULL(a->items);
    DTUNITTEST_ASSERT_INT(a->items[0], ==, 0);
    DTUNITTEST_ASSERT_INT(a->items[1], ==, 0);
    DTUNITTEST_ASSERT_INT(a->items[2], ==, 0);

    dtarray_float_dispose(a);

cleanup:
    return dterr;
}

static dterr_t*
test_dtarray_float_pack_length_math(void)
{
    dterr_t* dterr = NULL;

    dtarray_float_t* a = NULL;
    DTUNITTEST_ASSERT_NULL(dtarray_float_create(&a, 5));

    int32_t L = dtarray_float_pack_length(a);
    DTUNITTEST_ASSERT_INT(L, ==, (1 + 5) * 4);

    dtarray_float_dispose(a);

cleanup:
    return dterr;
}

static dterr_t*
test_dtarray_float_pack_and_unpack_roundtrip(void)
{
    dterr_t* dterr = NULL;

    dtarray_float_t* src = NULL;
    DTUNITTEST_ASSERT_NULL(dtarray_float_create(&src, 3));
    src->items[0] = 10;
    src->items[1] = -1;
    src->items[2] = 0x12345678;

    const int32_t need = dtarray_float_pack_length(src);
    DTUNITTEST_ASSERT_INT(need, >, 0);

    uint8_t* buf = (uint8_t*)calloc((size_t)need + 8u, 1u); // headroom
    DTUNITTEST_ASSERT_NOT_NULL(buf);

    // Pack at offset 0
    int32_t wrote = dtarray_float_pack(src, buf, 0, need);
    DTUNITTEST_ASSERT_INT(wrote, ==, need);

    // Unpack into a fresh dest with matching capacity
    dtarray_float_t* dst = NULL;
    DTUNITTEST_ASSERT_NULL(dtarray_float_create(&dst, 3));
    int32_t read = dtarray_float_unpack(dst, buf, 0, need);
    DTUNITTEST_ASSERT_INT(read, ==, need);

    DTUNITTEST_ASSERT_INT(dst->items[0], ==, 10);
    DTUNITTEST_ASSERT_INT(dst->items[1], ==, -1);
    DTUNITTEST_ASSERT_INT(dst->items[2], ==, 0x12345678);

    dtarray_float_dispose(dst);
    dtarray_float_dispose(src);
    free(buf);

cleanup:
    return dterr;
}

static dterr_t*
test_dtarray_float_unpack_into_smaller_capacity_is_truncated(void)
{
    dterr_t* dterr = NULL;

    // Source with 3 elements
    dtarray_float_t* src = NULL;
    DTUNITTEST_ASSERT_NULL(dtarray_float_create(&src, 3));
    src->items[0] = 111;
    src->items[1] = 222;
    src->items[2] = 333;

    const int32_t need = dtarray_float_pack_length(src);
    DTUNITTEST_ASSERT_INT(need, >, 0);

    uint8_t* buf = (uint8_t*)malloc((size_t)need);
    DTUNITTEST_ASSERT_NOT_NULL(buf);

    int32_t wrote = dtarray_float_pack(src, buf, 0, need);
    DTUNITTEST_ASSERT_INT(wrote, ==, need);

    // Destination with only 2 slots; impl should store first 2 and skip the 3rd
    dtarray_float_t* dst2 = NULL;
    DTUNITTEST_ASSERT_NULL(dtarray_float_create(&dst2, 2));

    int32_t read = dtarray_float_unpack(dst2, buf, 0, need);
    DTUNITTEST_ASSERT_INT(read, ==, need);
    DTUNITTEST_ASSERT_INT(dst2->items[0], ==, 111);
    DTUNITTEST_ASSERT_INT(dst2->items[1], ==, 222); // 333 should be ignored

    dtarray_float_dispose(dst2);
    dtarray_float_dispose(src);
    free(buf);

cleanup:
    return dterr;
}

// -----------------------------------------------------------------------------
// Runner

void
test_dtcore_dtarray_float(DTUNITTEST_SUITE_ARGS)
{
    DTUNITTEST_RUN_TEST(test_dtarray_float_create_bad_args);
    DTUNITTEST_RUN_TEST(test_dtarray_float_create_ok_and_dispose);
    DTUNITTEST_RUN_TEST(test_dtarray_float_pack_length_math);
    DTUNITTEST_RUN_TEST(test_dtarray_float_pack_and_unpack_roundtrip);
    DTUNITTEST_RUN_TEST(test_dtarray_float_unpack_into_smaller_capacity_is_truncated);
}
