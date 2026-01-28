#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dterr.h>
#include <dtcore/dtlog.h>
#include <dtcore/dtunittest.h>

#include <dtcore/dtheaper.h>
#include <dtcore_tests.h>

#define TAG "test_dtheaper"

/*
 * Local copy of the header layout so we can build “fake” headers
 * for negative / corner-case tests. This must match dtheaper.c.
 */
typedef struct dtheaper_header_test_t
{
    uint32_t magic1;
    int32_t length;
    int32_t length_negated;
    uint32_t magic2;
} dtheaper_header_test_t;

//------------------------------------------------------------------------
// Example 1: simple roundtrip allocate → fill → free
static dterr_t*
test_dtcore_dtheaper_example_roundtrip(void)
{
    dterr_t* dterr = NULL;

    void* payload = NULL;
    uint8_t* bytes = NULL;
    const int32_t length = 8;

    DTERR_C(dtheaper_alloc(length, "example_roundtrip", &payload));
    DTUNITTEST_ASSERT_NOT_NULL(payload);

    bytes = (uint8_t*)payload;

    // Write an initial pattern
    for (int32_t i = 0; i < length; ++i)
    {
        bytes[i] = (uint8_t)i;
    }

    // Fill with a constant value and verify
    DTERR_C(dtheaper_fill(payload, (char)0xAA));

    for (int32_t i = 0; i < length; ++i)
    {
        DTUNITTEST_ASSERT_INT(bytes[i], ==, (int32_t)0xAA);
    }

cleanup:
    if (payload != NULL)
    {
        dtheaper_free(payload);
        payload = NULL;
    }
    return dterr;
}

//------------------------------------------------------------------------
// Example 2: multiple allocations with different sizes & fill values
static dterr_t*
test_dtcore_dtheaper_example_multiple_allocs(void)
{
    dterr_t* dterr = NULL;

    void* p1 = NULL;
    void* p2 = NULL;
    void* p3 = NULL;

    const int32_t len1 = 4;
    const int32_t len2 = 7;
    const int32_t len3 = 1;

    DTERR_C(dtheaper_alloc(len1, "example_multi_1", &p1));
    DTERR_C(dtheaper_alloc(len2, "example_multi_2", &p2));
    DTERR_C(dtheaper_alloc(len3, "example_multi_3", &p3));

    DTUNITTEST_ASSERT_NOT_NULL(p1);
    DTUNITTEST_ASSERT_NOT_NULL(p2);
    DTUNITTEST_ASSERT_NOT_NULL(p3);

    DTERR_C(dtheaper_fill(p1, 'A'));
    DTERR_C(dtheaper_fill(p2, 'B'));
    DTERR_C(dtheaper_fill(p3, 'C'));

    for (int32_t i = 0; i < len1; ++i)
        DTUNITTEST_ASSERT_INT(((uint8_t*)p1)[i], ==, (int32_t)'A');
    for (int32_t i = 0; i < len2; ++i)
        DTUNITTEST_ASSERT_INT(((uint8_t*)p2)[i], ==, (int32_t)'B');
    for (int32_t i = 0; i < len3; ++i)
        DTUNITTEST_ASSERT_INT(((uint8_t*)p3)[i], ==, (int32_t)'C');

cleanup:
    if (p1 != NULL)
        dtheaper_free(p1);
    if (p2 != NULL)
        dtheaper_free(p2);
    if (p3 != NULL)
        dtheaper_free(p3);
    return dterr;
}

//------------------------------------------------------------------------
// Core test 01: alloc returns a payload area that is writeable
// and doesn't corrupt adjacent memory or crash.
static dterr_t*
test_dtcore_dtheaper_01_alloc_and_fill_basic(void)
{
    dterr_t* dterr = NULL;

    const int32_t length = 16;
    void* payload = NULL;
    uint8_t* bytes = NULL;

    DTERR_C(dtheaper_alloc(length, "basic_alloc", &payload));
    DTUNITTEST_ASSERT_NOT_NULL(payload);

    bytes = (uint8_t*)payload;

    // Write a simple pattern to the payload
    for (int32_t i = 0; i < length; ++i)
    {
        bytes[i] = (uint8_t)(0x10 + i);
    }

    // Verify pattern before calling fill
    for (int32_t i = 0; i < length; ++i)
    {
        DTUNITTEST_ASSERT_INT(bytes[i], ==, (int32_t)(0x10 + i));
    }

    // Now fill and verify again
    DTERR_C(dtheaper_fill(payload, (char)0x5A)); /* 'Z'ish */

    for (int32_t i = 0; i < length; ++i)
    {
        DTUNITTEST_ASSERT_INT(bytes[i], ==, (int32_t)0x5A);
    }

cleanup:
    if (payload != NULL)
        dtheaper_free(payload);
    return dterr;
}

//------------------------------------------------------------------------
// Core test 02: dtheaper_fill() rejects a buffer whose header does not
// look like a valid dtheaper header, and does not modify the payload.
static dterr_t*
test_dtcore_dtheaper_02_fill_invalid_header_fails_and_does_not_touch_payload(void)
{
    dterr_t* dterr = NULL;

    dtheaper_header_test_t* header = NULL;
    uint8_t* payload = NULL;

    const int32_t length = 8;
    const uint8_t sentinel = 0xAB;

    header = (dtheaper_header_test_t*)malloc(sizeof(dtheaper_header_test_t) + (size_t)length);
    DTUNITTEST_ASSERT_NOT_NULL(header);

    payload = (uint8_t*)(header + 1);

    // Intentionally set an INVALID header:
    //  - magics do not match dtheaper.c
    //  - BUT length and length_negated are consistent and > 0 so
    //    the only failure reason is the magic check.
    header->magic1 = 0u;
    header->magic2 = 0u;
    header->length = length;
    header->length_negated = -length;

    // Initialize payload to a known value
    for (int32_t i = 0; i < length; ++i)
        payload[i] = sentinel;

    dterr = dtheaper_fill((void*)payload, (char)0x11);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_FAIL);

    // Verify payload is UNCHANGED (no memset was performed)
    for (int32_t i = 0; i < length; ++i)
    {
        DTUNITTEST_ASSERT_INT(payload[i], ==, (int32_t)sentinel);
    }

cleanup:
    if (header != NULL)
        free(header);
    return dterr;
}

//------------------------------------------------------------------------
// Core test 03: dtheaper_free(NULL) is a no-op (should not crash or
// attempt to touch anything).
static dterr_t*
test_dtcore_dtheaper_03_free_null_is_noop(void)
{
    dterr_t* dterr = NULL;

    // Nothing to set up; just ensure this doesn't explode.
    dtheaper_free(NULL);

    return dterr;
}

//------------------------------------------------------------------------
// Core test 04: dtheaper_free() given a buffer with an invalid header
// must silently ignore it and *not* free the underlying allocation.
// We can't directly see the free(), but we can at least confirm it
// doesn't overwrite our header or crash.
static dterr_t*
test_dtcore_dtheaper_04_free_invalid_header_is_silent(void)
{
    dterr_t* dterr = NULL;

    dtheaper_header_test_t* header = NULL;
    void* payload = NULL;

    const int32_t length = 4;

    header = (dtheaper_header_test_t*)malloc(sizeof(dtheaper_header_test_t) + (size_t)length);
    DTUNITTEST_ASSERT_NOT_NULL(header);

    payload = (void*)(header + 1);

    // Mark header with a recognizable pattern
    header->magic1 = 0x11111111u;
    header->magic2 = 0x22222222u;
    header->length = length;
    header->length_negated = -length;

    dtheaper_free(payload);

    // If free() had treated this as valid, it would have zeroed the header
    // and then called free(header), making header unusable. By contract,
    // we expect it to *ignore* invalid headers, so header contents should
    // still be intact at this point.
    DTUNITTEST_ASSERT_INT((int32_t)header->magic1, ==, (int32_t)0x11111111u);
    DTUNITTEST_ASSERT_INT((int32_t)header->magic2, ==, (int32_t)0x22222222u);
    DTUNITTEST_ASSERT_INT(header->length, ==, length);
    DTUNITTEST_ASSERT_INT(header->length_negated, ==, -length);

cleanup:
    if (header != NULL)
        free(header);
    return dterr;
}

//------------------------------------------------------------------------
// Suite entry point
void
test_dtcore_dtheaper(DTUNITTEST_SUITE_ARGS)
{
    /* Simple “example” tests first */
    DTUNITTEST_RUN_TEST(test_dtcore_dtheaper_example_roundtrip);
    DTUNITTEST_RUN_TEST(test_dtcore_dtheaper_example_multiple_allocs);

    /* Core / corner-case tests */
    DTUNITTEST_RUN_TEST(test_dtcore_dtheaper_01_alloc_and_fill_basic);
    DTUNITTEST_RUN_TEST(test_dtcore_dtheaper_02_fill_invalid_header_fails_and_does_not_touch_payload);
    DTUNITTEST_RUN_TEST(test_dtcore_dtheaper_03_free_null_is_noop);
    DTUNITTEST_RUN_TEST(test_dtcore_dtheaper_04_free_invalid_header_is_silent);
}
