// test_dtbuffer.c
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dtbuffer.h>
#include <dtcore/dterr.h>
#include <dtcore/dtunittest.h>
#include <dtcore_tests.h>

// New, short "example" tests first. These illustrate common usage patterns.
//----------------------------------------------------------------------------

// Small buffer that wraps external storage; shows read/write roundtrip.
dterr_t*
test_dtbuffer_example_wrap_basic(void)
{
    dterr_t* dterr = NULL;

    uint8_t external[4] = { 0 };
    dtbuffer_t b = { 0 };

    DTUNITTEST_ASSERT_NULL(dtbuffer_wrap(&b, external, (int32_t)sizeof external));

    // write, then read back
    for (size_t i = 0; i < sizeof external; ++i)
        ((uint8_t*)b.payload)[i] = (uint8_t)(i + 1);
    DTUNITTEST_ASSERT_UINT8(external[0], ==, 1);
    DTUNITTEST_ASSERT_UINT8(external[3], ==, 4);

    // dispose() on wrapped buffers should only zero fields, not free external[]
    dtbuffer_dispose(&b);
    DTUNITTEST_ASSERT_UINT8(external[0], ==, 1); // still intact

cleanup:
    return dterr;
}
//------------------------------------------------------------------------
// Example of create() → use → dispose(). Payload is contiguous after header.
dterr_t*
test_dtbuffer_example_create_use_dispose(void)
{
    dterr_t* dterr = NULL;

    dtbuffer_t* b = NULL;
    const int32_t len = 16;

    DTUNITTEST_ASSERT_NULL(dtbuffer_create(&b, len));
    DTUNITTEST_ASSERT_NOT_NULL(b);

    // payload immediately follows the struct
    uint8_t* expected = (uint8_t*)b + sizeof *b;
    DTUNITTEST_ASSERT_PTR(b->payload, ==, expected);

    memset(b->payload, 0xCC, (size_t)len);
    for (int32_t i = 0; i < len; ++i)
        DTUNITTEST_ASSERT_UINT8(((uint8_t*)b->payload)[i], ==, 0xCC);

    dtbuffer_dispose(b); // frees because it was created

cleanup:
    return dterr;
}
//------------------------------------------------------------------------
// Zero-length buffers are allowed; payload still points past the struct.
dterr_t*
test_dtbuffer_example_zero_length_buffer(void)
{
    dterr_t* dterr = NULL;

    dtbuffer_t* b = NULL;
    DTUNITTEST_ASSERT_NULL(dtbuffer_create(&b, 0));
    DTUNITTEST_ASSERT_NOT_NULL(b);
    DTUNITTEST_ASSERT_UINT32(b->length, ==, 0u);

    uint8_t* expected = (uint8_t*)b + sizeof *b;
    DTUNITTEST_ASSERT_PTR(b->payload, ==, expected);

    dtbuffer_dispose(b);

cleanup:
    return dterr;
}

//------------------------------------------------------------------------------
// Numbered tests (keep/improve original coverage and add a few edge cases).
//------------------------------------------------------------------------------

// 01 — wrap() sets fields and WRAPPED flag.
dterr_t*
test_dtbuffer_01_wrap_sets_fields(void)
{
    dterr_t* dterr = NULL;

    dtbuffer_t b;
    memset(&b, 0xA5, sizeof b);

    uint8_t external[32] = { 0 };
    DTUNITTEST_ASSERT_NULL(dtbuffer_wrap(&b, external, (int32_t)sizeof external));

    DTUNITTEST_ASSERT_PTR(b.payload, ==, external);
    DTUNITTEST_ASSERT_UINT32(b.length, ==, (uint32_t)sizeof external);
    DTUNITTEST_ASSERT_UINT32((uint32_t)(b.flags & DTBUFFER_WRAPPED), ==, (uint32_t)DTBUFFER_WRAPPED);

cleanup:
    return dterr;
}
//------------------------------------------------------------------------
// 02 — create() allocates, sets CREATED flag, and payload is after struct.
dterr_t*
test_dtbuffer_02_create_allocates_and_sets_payload(void)
{
    dterr_t* dterr = NULL;

    const int32_t want_len = 64;
    dtbuffer_t* b = NULL;

    DTUNITTEST_ASSERT_NULL(dtbuffer_create(&b, want_len));
    DTUNITTEST_ASSERT_NOT_NULL(b);

    DTUNITTEST_ASSERT_UINT32(b->length, ==, (uint32_t)want_len);
    DTUNITTEST_ASSERT_UINT32((uint32_t)(b->flags & DTBUFFER_CREATED), ==, (uint32_t)DTBUFFER_CREATED);
    DTUNITTEST_ASSERT_NOT_NULL(b->payload);

    uint8_t* expected_payload = (uint8_t*)b + sizeof *b;
    DTUNITTEST_ASSERT_PTR(b->payload, ==, expected_payload);

    memset(b->payload, 0x3C, (size_t)want_len);
    for (int32_t i = 0; i < want_len; ++i)
        DTUNITTEST_ASSERT_UINT8(((uint8_t*)b->payload)[i], ==, 0x3C);

    dtbuffer_dispose(b);

cleanup:
    return dterr;
}
//------------------------------------------------------------------------
// 03 — create(0) is valid; payload points past struct.
dterr_t*
test_dtbuffer_03_create_zero_length_is_valid(void)
{
    dterr_t* dterr = NULL;

    dtbuffer_t* b = NULL;
    DTUNITTEST_ASSERT_NULL(dtbuffer_create(&b, 0));
    DTUNITTEST_ASSERT_NOT_NULL(b);

    DTUNITTEST_ASSERT_UINT32(b->length, ==, 0u);
    DTUNITTEST_ASSERT_UINT32((uint32_t)(b->flags & DTBUFFER_CREATED), ==, (uint32_t)DTBUFFER_CREATED);

    uint8_t* expected_payload = (uint8_t*)b + sizeof *b;
    DTUNITTEST_ASSERT_PTR(b->payload, ==, expected_payload);

    dtbuffer_dispose(b);

cleanup:
    return dterr;
}
//------------------------------------------------------------------------
// 04 — dispose() on wrapped: zero struct, do NOT free the external memory.
dterr_t*
test_dtbuffer_04_dispose_wrapped_zeros_only(void)
{
    dterr_t* dterr = NULL;

    dtbuffer_t b;
    memset(&b, 0xA5, sizeof b);

    uint8_t external[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    DTUNITTEST_ASSERT_NULL(dtbuffer_wrap(&b, external, (int32_t)sizeof external));

    dtbuffer_dispose(&b);

    DTUNITTEST_ASSERT_NULL(b.payload);
    DTUNITTEST_ASSERT_UINT32(b.length, ==, 0u);
    DTUNITTEST_ASSERT_UINT32(b.flags, ==, 0u);
    DTUNITTEST_ASSERT_UINT8(external[0], ==, 1);
    DTUNITTEST_ASSERT_UINT8(external[7], ==, 8);

cleanup:
    return dterr;
}
//------------------------------------------------------------------------
// 05 — dispose(NULL) is safe no-op.
dterr_t*
test_dtbuffer_05_dispose_null_is_safe(void)
{
    dterr_t* dterr = NULL;

    dtbuffer_dispose(NULL);

    return dterr;
}
//------------------------------------------------------------------------
// 06 — wrap() with negative length should return an error.
dterr_t*
test_dtbuffer_06_wrap_negative_length_is_error(void)
{
    dterr_t* dterr = NULL;

    dtbuffer_t b = { 0 };
    uint8_t bytes[1] = { 0 };
    dterr = dtbuffer_wrap(&b, bytes, -1);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_BADARG);

cleanup:
    return dterr;
}
//------------------------------------------------------------------------
// 07 — wrap() with NULL payload should return an error (argument check).
dterr_t*
test_dtbuffer_07_wrap_null_payload_is_error(void)
{
    dterr_t* dterr = NULL;

    dtbuffer_t b = { 0 };
    dterr = dtbuffer_wrap(&b, NULL, 4);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_ARGUMENT_NULL);

cleanup:
    return dterr;
}
//------------------------------------------------------------------------
// 08 — create() with NULL out-parameter should return an error (argument check).
dterr_t*
test_dtbuffer_08_create_null_outparam_is_error(void)
{
    dterr_t* dterr = NULL;

    dterr = dtbuffer_create(NULL, 8);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_ARGUMENT_NULL);

cleanup:
    return dterr;
}

//------------------------------------------------------------------------------
// Suite entry point (kept same symbol for drop-in compatibility).
void
test_dtcore_dtbuffer(DTUNITTEST_SUITE_ARGS)
{
    // Examples first
    DTUNITTEST_RUN_TEST(test_dtbuffer_example_wrap_basic);
    DTUNITTEST_RUN_TEST(test_dtbuffer_example_create_use_dispose);
    DTUNITTEST_RUN_TEST(test_dtbuffer_example_zero_length_buffer);

    // Numbered coverage + edges
    DTUNITTEST_RUN_TEST(test_dtbuffer_01_wrap_sets_fields);
    DTUNITTEST_RUN_TEST(test_dtbuffer_02_create_allocates_and_sets_payload);
    DTUNITTEST_RUN_TEST(test_dtbuffer_03_create_zero_length_is_valid);
    DTUNITTEST_RUN_TEST(test_dtbuffer_04_dispose_wrapped_zeros_only);
    DTUNITTEST_RUN_TEST(test_dtbuffer_05_dispose_null_is_safe);
    DTUNITTEST_RUN_TEST(test_dtbuffer_06_wrap_negative_length_is_error);
    DTUNITTEST_RUN_TEST(test_dtbuffer_07_wrap_null_payload_is_error);
    DTUNITTEST_RUN_TEST(test_dtbuffer_08_create_null_outparam_is_error);
}
