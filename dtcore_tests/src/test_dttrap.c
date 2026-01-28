// dttrap_unittest.c
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dterr.h>
#include <dtcore/dtlog.h>
#include <dtcore/dttrap.h>

#include <dtcore/dtunittest.h>

// --- test helpers ------------------------------------------------------------

static dterr_t*
test_dttrap_cb(dttrap_t* self, void* user_ctx)
{
    (void)self;
    int* hits = (int*)user_ctx;
    if (hits)
        (*hits)++;
    return NULL;
}

// --- tests -------------------------------------------------------------------

dterr_t*
test_dtcore_dttrap_init_defaults(void)
{
    dterr_t* dterr = NULL;

    dttrap_t trap;
    memset(&trap, 0xA5, sizeof(trap));          // poison to ensure init really sets fields
    DTUNITTEST_ASSERT_NULL(dttrap_init(&trap)); // should return NULL meaning "ok"

    DTUNITTEST_ASSERT_UINT32(trap.count, ==, 0u);
    DTUNITTEST_ASSERT_UINT32(trap.stop_trapping_after_count, ==, 10u);
    DTUNITTEST_ASSERT_NULL(trap.lines);
    DTUNITTEST_ASSERT_NULL(trap.callback);
    // user_context is unspecified; just ensure not crashing to read
    (void)trap.callback_user_context;

cleanup:
    return dterr;
}

dterr_t*
test_dtcore_dttrap_set_callback_and_fire_on_limit(void)
{
    dterr_t* dterr = NULL;

    dttrap_t trap;
    DTUNITTEST_ASSERT_NULL(dttrap_init(&trap));

    int hits = 0;
    DTUNITTEST_ASSERT_NULL(dttrap_set_callback(&trap, test_dttrap_cb, &hits));

    trap.stop_trapping_after_count = 3;

    DTUNITTEST_ASSERT_UINT32(trap.count, ==, 0u);
    DTUNITTEST_ASSERT_NULL(dttrap_count(&trap));
    DTUNITTEST_ASSERT_UINT32(trap.count, ==, 1u);
    DTUNITTEST_ASSERT_INT(hits, ==, 0);

    DTUNITTEST_ASSERT_NULL(dttrap_count(&trap));
    DTUNITTEST_ASSERT_UINT32(trap.count, ==, 2u);
    DTUNITTEST_ASSERT_INT(hits, ==, 0);

    // This one should fire (exact equality)
    DTUNITTEST_ASSERT_NULL(dttrap_count(&trap));
    DTUNITTEST_ASSERT_UINT32(trap.count, ==, 3u);
    DTUNITTEST_ASSERT_INT(hits, ==, 1);

    // Further increments should NOT fire again
    DTUNITTEST_ASSERT_NULL(dttrap_count(&trap));
    DTUNITTEST_ASSERT_UINT32(trap.count, ==, 4u);
    DTUNITTEST_ASSERT_INT(hits, ==, 1);

    dttrap_dispose(&trap);
cleanup:
    return dterr;
}

dterr_t*
test_dtcore_dttrap_no_fire_before_limit(void)
{
    dterr_t* dterr = NULL;

    dttrap_t trap;
    DTUNITTEST_ASSERT_NULL(dttrap_init(&trap));
    trap.stop_trapping_after_count = 5;

    int hits = 0;
    DTUNITTEST_ASSERT_NULL(dttrap_set_callback(&trap, test_dttrap_cb, &hits));

    for (uint32_t i = 0; i < 4; ++i)
    {
        DTUNITTEST_ASSERT_NULL(dttrap_count(&trap));
    }
    DTUNITTEST_ASSERT_UINT32(trap.count, ==, 4u);
    DTUNITTEST_ASSERT_INT(hits, ==, 0);

    dttrap_dispose(&trap);
cleanup:
    return dterr;
}

dterr_t*
test_dtcore_dttrap_append_accumulates_until_limit(void)
{
    dterr_t* dterr = NULL;

    dttrap_t trap;
    DTUNITTEST_ASSERT_NULL(dttrap_init(&trap));
    trap.stop_trapping_after_count = 3;

    // Append before any counts
    DTUNITTEST_ASSERT_NULL(dttrap_append(&trap, "first %d", 1));
    DTUNITTEST_ASSERT_NOT_NULL(trap.lines);
    size_t len1 = strlen(trap.lines);
    DTUNITTEST_ASSERT_UINT32(len1, >, 0u);

    DTUNITTEST_ASSERT_NULL(dttrap_count(&trap)); // count = 1

    // Append again
    DTUNITTEST_ASSERT_NULL(dttrap_append(&trap, "second %s", "two"));
    DTUNITTEST_ASSERT_NOT_NULL(trap.lines);
    size_t len2 = strlen(trap.lines);
    DTUNITTEST_ASSERT_UINT32(len2, >, len1);

    // Hit the threshold
    DTUNITTEST_ASSERT_NULL(dttrap_count(&trap)); // count = 2
    DTUNITTEST_ASSERT_NULL(dttrap_count(&trap)); // count = 3 => fires (if callback set). No callback here.

    // Appends at/after limit must be ignored (no growth)
    char* before_ptr = trap.lines;
    size_t before_len = strlen(trap.lines);

    DTUNITTEST_ASSERT_NULL(dttrap_append(&trap, "IGNORED %d", 99));
    // The implementation returns NULL always for "ok", so we check content didn't grow:
    DTUNITTEST_ASSERT_PTR(trap.lines, ==, before_ptr);
    DTUNITTEST_ASSERT_UINT32(strlen(trap.lines), ==, before_len);

    dttrap_dispose(&trap);
cleanup:
    return dterr;
}

dterr_t*
test_dtcore_dttrap_fire_only_once_even_if_exceeded(void)
{
    dterr_t* dterr = NULL;

    dttrap_t trap;
    DTUNITTEST_ASSERT_NULL(dttrap_init(&trap));
    trap.stop_trapping_after_count = 2;

    int hits = 0;
    DTUNITTEST_ASSERT_NULL(dttrap_set_callback(&trap, test_dttrap_cb, &hits));

    DTUNITTEST_ASSERT_NULL(dttrap_count(&trap)); // 1
    DTUNITTEST_ASSERT_INT(hits, ==, 0);

    DTUNITTEST_ASSERT_NULL(dttrap_count(&trap)); // 2 -> fire
    DTUNITTEST_ASSERT_INT(hits, ==, 1);

    // Blast past the limit; should not fire again
    DTUNITTEST_ASSERT_NULL(dttrap_count(&trap)); // 3
    DTUNITTEST_ASSERT_NULL(dttrap_count(&trap)); // 4
    DTUNITTEST_ASSERT_INT(hits, ==, 1);

    dttrap_dispose(&trap);
cleanup:
    return dterr;
}

dterr_t*
test_dtcore_dttrap_debug_safe_after_append(void)
{
    dterr_t* dterr = NULL;

    dttrap_t trap;
    DTUNITTEST_ASSERT_NULL(dttrap_init(&trap));

    // Ensure lines not NULL before debug to avoid UB with "%s"
    DTUNITTEST_ASSERT_NULL(dttrap_append(&trap, "hello %s", "world"));
    DTUNITTEST_ASSERT_NOT_NULL(trap.lines);

    // Should not crash
    DTUNITTEST_ASSERT_NULL(dttrap_debug(&trap));

    dttrap_dispose(&trap);
cleanup:
    return dterr;
}

dterr_t*
test_dtcore_dttrap_dispose_zeros(void)
{
    dterr_t* dterr = NULL;

    dttrap_t trap;
    DTUNITTEST_ASSERT_NULL(dttrap_init(&trap));
    trap.stop_trapping_after_count = 1;
    DTUNITTEST_ASSERT_NULL(dttrap_append(&trap, "something"));

    // dispose should free lines and zero the struct
    dttrap_dispose(&trap);

    // We can only safely check a few fields after memset(0)
    DTUNITTEST_ASSERT_UINT32(trap.count, ==, 0u);
    DTUNITTEST_ASSERT_UINT32(trap.stop_trapping_after_count, ==, 0u);
    DTUNITTEST_ASSERT_NULL(trap.lines);
    DTUNITTEST_ASSERT_NULL(trap.callback);
    DTUNITTEST_ASSERT_NULL(trap.callback_user_context);

cleanup:
    return dterr;
}

dterr_t*
test_dtcore_dttrap_set_callback_stores_context(void)
{
    dterr_t* dterr = NULL;

    dttrap_t trap;
    DTUNITTEST_ASSERT_NULL(dttrap_init(&trap));

    int ctx_value = 42;
    DTUNITTEST_ASSERT_NULL(dttrap_set_callback(&trap, test_dttrap_cb, &ctx_value));
    DTUNITTEST_ASSERT_PTR(trap.callback, ==, test_dttrap_cb);
    DTUNITTEST_ASSERT_PTR(trap.callback_user_context, ==, &ctx_value);

    dttrap_dispose(&trap);
cleanup:
    return dterr;
}

// ------------------------------------------------------------------------------
void
test_dtcore_dttrap(DTUNITTEST_SUITE_ARGS)
{
    DTUNITTEST_RUN_TEST(test_dtcore_dttrap_init_defaults);
    DTUNITTEST_RUN_TEST(test_dtcore_dttrap_set_callback_and_fire_on_limit);
    DTUNITTEST_RUN_TEST(test_dtcore_dttrap_no_fire_before_limit);
    DTUNITTEST_RUN_TEST(test_dtcore_dttrap_fire_only_once_even_if_exceeded);
    DTUNITTEST_RUN_TEST(test_dtcore_dttrap_append_accumulates_until_limit);

    DTUNITTEST_RUN_TEST(test_dtcore_dttrap_debug_safe_after_append);
    DTUNITTEST_RUN_TEST(test_dtcore_dttrap_dispose_zeros);
    DTUNITTEST_RUN_TEST(test_dtcore_dttrap_set_callback_stores_context);
}
