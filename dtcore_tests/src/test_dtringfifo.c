#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <dtcore/dterr.h>
#include <dtcore/dtringfifo.h>
#include <dtcore/dtunittest.h>
#include <dtcore_tests.h>

// Example: basic push/pop without wrap-around.
static dterr_t*
test_dtcore_dtringfifo_example_basic_push_pop(void)
{
    dterr_t* dterr = NULL;

    dtringfifo_t fifo = { 0 };
    uint8_t buffer[8] = { 0 };
    dtringfifo_config_t cfg = {
        .buffer = buffer,
        .capacity = (int32_t)sizeof(buffer),
    };

    DTERR_C(dtringfifo_init(&fifo));
    DTERR_C(dtringfifo_configure(&fifo, &cfg));

    // Push a small payload.
    uint8_t src[] = { 10, 11, 12 };
    int32_t pushed = dtringfifo_push(&fifo, src, (int32_t)sizeof(src));
    DTUNITTEST_ASSERT_INT(pushed, ==, 3);

    // Pop it back out.
    uint8_t dest[3] = { 0 };
    int32_t popped = dtringfifo_pop(&fifo, dest, (int32_t)sizeof(dest));
    DTUNITTEST_ASSERT_INT(popped, ==, 3);
    DTUNITTEST_ASSERT_EQUAL_BYTES(dest, src, sizeof(src));

    // After emptying, further pops should yield zero bytes.
    popped = dtringfifo_pop(&fifo, dest, (int32_t)sizeof(dest));
    DTUNITTEST_ASSERT_INT(popped, ==, 0);

cleanup:
    return dterr;
}
// ------------------------------------------------------------------------

// Example: wrap-around behaviour when producer/consumer interleave.
static dterr_t*
test_dtcore_dtringfifo_example_wraparound_usage(void)
{
    dterr_t* dterr = NULL;

    dtringfifo_t fifo = { 0 };
    uint8_t buffer[5] = { 0 }; // capacity=5 -> up to 4 usable bytes
    dtringfifo_config_t cfg = {
        .buffer = buffer,
        .capacity = (int32_t)sizeof(buffer),
    };

    DTERR_C(dtringfifo_init(&fifo));
    DTERR_C(dtringfifo_configure(&fifo, &cfg));

    // Fill most of the buffer.
    uint8_t first[] = { 1, 2, 3 };
    int32_t pushed = dtringfifo_push(&fifo, first, (int32_t)sizeof(first));
    DTUNITTEST_ASSERT_INT(pushed, ==, 3);

    // Consume two bytes so tail moves forward (but head stays near the end).
    uint8_t tmp[2] = { 0 };
    int32_t popped = dtringfifo_pop(&fifo, tmp, 2);
    DTUNITTEST_ASSERT_INT(popped, ==, 2);

    // Push additional bytes to force the head to wrap to the beginning.
    uint8_t second[] = { 4, 5, 6 };
    pushed = dtringfifo_push(&fifo, second, (int32_t)sizeof(second));
    DTUNITTEST_ASSERT_INT(pushed, ==, 3);

    // Pop all remaining bytes and verify sequence (FIFO order).
    uint8_t dest[4] = { 0 };
    popped = dtringfifo_pop(&fifo, dest, (int32_t)sizeof(dest));
    DTUNITTEST_ASSERT_INT(popped, ==, 4);

    // We pushed [1,2,3], popped [1,2], then pushed [4,5,6].
    // Remaining sequence should be [3,4,5,6].
    uint8_t expected[] = { 3, 4, 5, 6 };
    DTUNITTEST_ASSERT_EQUAL_BYTES(dest, expected, sizeof(expected));

    // Buffer should now be empty.
    popped = dtringfifo_pop(&fifo, dest, (int32_t)sizeof(dest));
    DTUNITTEST_ASSERT_INT(popped, ==, 0);

cleanup:
    return dterr;
}
// ------------------------------------------------------------------------

// Example: tracking dropped bytes when producer outpaces consumer,
// using the return value of dtringfifo_push().
static dterr_t*
test_dtcore_dtringfifo_example_dropped_bytes_monitoring(void)
{
    dterr_t* dterr = NULL;

    dtringfifo_t fifo = { 0 };
    uint8_t buffer[4] = { 0 }; // capacity=4 -> up to 3 usable bytes
    dtringfifo_config_t cfg = {
        .buffer = buffer,
        .capacity = (int32_t)sizeof(buffer),
    };

    DTERR_C(dtringfifo_init(&fifo));
    DTERR_C(dtringfifo_configure(&fifo, &cfg));

    int32_t total_dropped = 0;

    // Push more bytes than we can hold; the excess should be dropped.
    uint8_t src1[] = { 10, 11, 12, 13, 14, 15 }; // len = 6
    int32_t requested1 = (int32_t)sizeof(src1);
    int32_t pushed1 = dtringfifo_push(&fifo, src1, requested1);

    // With capacity=4 and one slot kept empty, FIFO can hold up to 3 bytes.
    DTUNITTEST_ASSERT_INT(pushed1, ==, 3);
    total_dropped += (requested1 - pushed1);
    DTUNITTEST_ASSERT_INT(total_dropped, ==, 3);

    // Consume two bytes to make space, then push again.
    uint8_t dest[4] = { 0 };
    int32_t popped = dtringfifo_pop(&fifo, dest, 2);
    DTUNITTEST_ASSERT_INT(popped, ==, 2);

    uint8_t src2[] = { 20, 21, 22 }; // len = 3
    int32_t requested2 = (int32_t)sizeof(src2);
    int32_t pushed2 = dtringfifo_push(&fifo, src2, requested2);

    // After popping 2 from 3 stored, there's 1 left and 2 free => we can push 2.
    DTUNITTEST_ASSERT_INT(pushed2, ==, 2);
    total_dropped += (requested2 - pushed2);
    DTUNITTEST_ASSERT_INT(total_dropped, ==, 4);

    // Now read out all remaining bytes and verify they match FIFO order.
    popped = dtringfifo_pop(&fifo, dest, (int32_t)sizeof(dest));
    DTUNITTEST_ASSERT_INT(popped, >, 0);

    // We don't assert exact byte pattern here (because we're focused on drop
    // accounting via push return values), but we do assert subsequent pops are empty.
    int32_t popped2 = dtringfifo_pop(&fifo, dest, (int32_t)sizeof(dest));
    DTUNITTEST_ASSERT_INT(popped2, ==, 0);

cleanup:
    return dterr;
}
// ------------------------------------------------------------------------

// Null self pointer should produce an argument error on init.
static dterr_t*
test_dtcore_dtringfifo_01_init_null_self(void)
{
    dterr_t* dterr = NULL;

    dtringfifo_t* self = NULL;
    dterr = dtringfifo_init(self);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_ARGUMENT_NULL);

cleanup:
    return dterr;
}
// ------------------------------------------------------------------------

// Null self pointer should produce an argument error on configure.
static dterr_t*
test_dtcore_dtringfifo_02_configure_null_self(void)
{
    dterr_t* dterr = NULL;

    dtringfifo_t* self = NULL;
    uint8_t buffer[4] = { 0 };
    dtringfifo_config_t cfg = {
        .buffer = buffer,
        .capacity = 4,
    };

    dterr = dtringfifo_configure(self, &cfg);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_ARGUMENT_NULL);

cleanup:
    return dterr;
}
// ------------------------------------------------------------------------

// Null configuration pointer should produce an argument error.
static dterr_t*
test_dtcore_dtringfifo_03_configure_null_cfg(void)
{
    dterr_t* dterr = NULL;

    dtringfifo_t fifo = { 0 };
    dtringfifo_config_t* cfg = NULL;

    dterr = dtringfifo_configure(&fifo, cfg);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_ARGUMENT_NULL);

cleanup:
    return dterr;
}
// ------------------------------------------------------------------------

// Null buffer in configuration should produce an argument error.
static dterr_t*
test_dtcore_dtringfifo_04_configure_null_buffer(void)
{
    dterr_t* dterr = NULL;

    dtringfifo_t fifo = { 0 };
    dtringfifo_config_t cfg = {
        .buffer = NULL,
        .capacity = 4,
    };

    dterr = dtringfifo_configure(&fifo, &cfg);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_ARGUMENT_NULL);

cleanup:
    return dterr;
}
// ------------------------------------------------------------------------

// Zero capacity should be rejected with a bad-argument error.
static dterr_t*
test_dtcore_dtringfifo_05_configure_zero_capacity(void)
{
    dterr_t* dterr = NULL;

    dtringfifo_t fifo = { 0 };
    uint8_t buffer[1] = { 0 };
    dtringfifo_config_t cfg = {
        .buffer = buffer,
        .capacity = 0,
    };

    dterr = dtringfifo_configure(&fifo, &cfg);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_BADARG);

cleanup:
    return dterr;
}
// ------------------------------------------------------------------------

// Push and pop near the effective capacity (capacity-1 usable bytes).
static dterr_t*
test_dtcore_dtringfifo_06_push_pop_near_capacity(void)
{
    dterr_t* dterr = NULL;

    dtringfifo_t fifo = { 0 };
    uint8_t buffer[5] = { 0 }; // capacity=5 -> 4 usable bytes
    dtringfifo_config_t cfg = {
        .buffer = buffer,
        .capacity = 5,
    };

    DTERR_C(dtringfifo_init(&fifo));
    DTERR_C(dtringfifo_configure(&fifo, &cfg));

    uint8_t src[] = { 1, 2, 3, 4 };
    int32_t pushed = dtringfifo_push(&fifo, src, 4);
    DTUNITTEST_ASSERT_INT(pushed, ==, 4);

    uint8_t dest[4] = { 0 };
    int32_t popped = dtringfifo_pop(&fifo, dest, 4);
    DTUNITTEST_ASSERT_INT(popped, ==, 4);
    DTUNITTEST_ASSERT_EQUAL_BYTES(dest, src, sizeof(src));

    // After emptying, no further data should be available.
    popped = dtringfifo_pop(&fifo, dest, 4);
    DTUNITTEST_ASSERT_INT(popped, ==, 0);

cleanup:
    return dterr;
}
// ------------------------------------------------------------------------

// Popping more bytes than available should only return what exists.
static dterr_t*
test_dtcore_dtringfifo_07_pop_more_than_available(void)
{
    dterr_t* dterr = NULL;

    dtringfifo_t fifo = { 0 };
    uint8_t buffer[4] = { 0 };
    dtringfifo_config_t cfg = {
        .buffer = buffer,
        .capacity = 4,
    };

    DTERR_C(dtringfifo_init(&fifo));
    DTERR_C(dtringfifo_configure(&fifo, &cfg));

    uint8_t src[] = { 10, 20 };
    int32_t pushed = dtringfifo_push(&fifo, src, 2);
    DTUNITTEST_ASSERT_INT(pushed, ==, 2);

    uint8_t dest[5] = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA };
    int32_t popped = dtringfifo_pop(&fifo, dest, 5);
    DTUNITTEST_ASSERT_INT(popped, ==, 2);
    DTUNITTEST_ASSERT_EQUAL_BYTES(dest, src, sizeof(src));

    // After reading all data, nothing else should be available.
    popped = dtringfifo_pop(&fifo, dest, 5);
    DTUNITTEST_ASSERT_INT(popped, ==, 0);

cleanup:
    return dterr;
}
// ------------------------------------------------------------------------

// Reset should drop contents and allow reuse of the buffer.
static dterr_t*
test_dtcore_dtringfifo_08_reset_clears_state(void)
{
    dterr_t* dterr = NULL;

    dtringfifo_t fifo = { 0 };
    uint8_t buffer[4] = { 0 };
    dtringfifo_config_t cfg = {
        .buffer = buffer,
        .capacity = 4,
    };

    DTERR_C(dtringfifo_init(&fifo));
    DTERR_C(dtringfifo_configure(&fifo, &cfg));

    // Fill as much as we can, possibly dropping extra bytes.
    uint8_t src_full[] = { 1, 2, 3, 4, 5, 6 };
    (void)dtringfifo_push(&fifo, src_full, (int32_t)sizeof(src_full));

    // After reset, buffer should behave like freshly configured.
    dtringfifo_reset(&fifo);

    uint8_t src2[] = { 10, 11, 12 };
    int32_t pushed2 = dtringfifo_push(&fifo, src2, (int32_t)sizeof(src2));
    DTUNITTEST_ASSERT_INT(pushed2, ==, (int32_t)sizeof(src2));

    uint8_t dest[4] = { 0 };
    int32_t popped = dtringfifo_pop(&fifo, dest, 4);
    DTUNITTEST_ASSERT_INT(popped, ==, (int32_t)sizeof(src2));
    DTUNITTEST_ASSERT_EQUAL_BYTES(dest, src2, sizeof(src2));

    // And then it should again be empty.
    popped = dtringfifo_pop(&fifo, dest, 4);
    DTUNITTEST_ASSERT_INT(popped, ==, 0);

cleanup:
    return dterr;
}
// ------------------------------------------------------------------------

// Multiple small pushes and pops should behave like a FIFO queue.
static dterr_t*
test_dtcore_dtringfifo_09_interleaved_small_operations(void)
{
    dterr_t* dterr = NULL;

    dtringfifo_t fifo = { 0 };
    uint8_t buffer[6] = { 0 }; // capacity=6 -> 5 usable bytes
    dtringfifo_config_t cfg = {
        .buffer = buffer,
        .capacity = 6,
    };

    DTERR_C(dtringfifo_init(&fifo));
    DTERR_C(dtringfifo_configure(&fifo, &cfg));

    uint8_t a[] = { 1, 2 };
    uint8_t b[] = { 3 };
    uint8_t c[] = { 4, 5, 6 };

    (void)dtringfifo_push(&fifo, a, 2);
    (void)dtringfifo_push(&fifo, b, 1);

    uint8_t dest[6] = { 0 };
    int32_t popped = dtringfifo_pop(&fifo, dest, 2);
    DTUNITTEST_ASSERT_INT(popped, ==, 2);
    uint8_t expected1[] = { 1, 2 };
    DTUNITTEST_ASSERT_EQUAL_BYTES(dest, expected1, sizeof(expected1));

    (void)dtringfifo_push(&fifo, c, 3);

    popped = dtringfifo_pop(&fifo, dest, 4);
    DTUNITTEST_ASSERT_INT(popped, ==, 4);
    uint8_t expected2[] = { 3, 4, 5, 6 };
    DTUNITTEST_ASSERT_EQUAL_BYTES(dest, expected2, sizeof(expected2));

    // And now it should be empty again.
    popped = dtringfifo_pop(&fifo, dest, 4);
    DTUNITTEST_ASSERT_INT(popped, ==, 0);

cleanup:
    return dterr;
}
// ------------------------------------------------------------------------

// Suite entry point for dtringfifo tests.
void
test_dtcore_dtringfifo(DTUNITTEST_SUITE_ARGS)
{
    DTUNITTEST_RUN_TEST(test_dtcore_dtringfifo_example_basic_push_pop);
    DTUNITTEST_RUN_TEST(test_dtcore_dtringfifo_example_wraparound_usage);
    DTUNITTEST_RUN_TEST(test_dtcore_dtringfifo_example_dropped_bytes_monitoring);

    DTUNITTEST_RUN_TEST(test_dtcore_dtringfifo_01_init_null_self);
    DTUNITTEST_RUN_TEST(test_dtcore_dtringfifo_02_configure_null_self);
    DTUNITTEST_RUN_TEST(test_dtcore_dtringfifo_03_configure_null_cfg);
    DTUNITTEST_RUN_TEST(test_dtcore_dtringfifo_04_configure_null_buffer);
    DTUNITTEST_RUN_TEST(test_dtcore_dtringfifo_05_configure_zero_capacity);
    DTUNITTEST_RUN_TEST(test_dtcore_dtringfifo_06_push_pop_near_capacity);
    DTUNITTEST_RUN_TEST(test_dtcore_dtringfifo_07_pop_more_than_available);
    DTUNITTEST_RUN_TEST(test_dtcore_dtringfifo_08_reset_clears_state);
    DTUNITTEST_RUN_TEST(test_dtcore_dtringfifo_09_interleaved_small_operations);
}
