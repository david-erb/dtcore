#include <sys/types.h>

#include <inttypes.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>

#include <dtcore/dterr.h>
#include <dtcore/dtlog.h>
#include <dtcore/dtstr.h>
#include <dtcore/dtunittest.h>

#include <dtcore/dteventlogger.h>

#define TAG "test_dteventlogger"

// ---------------------------------------------------------------------------------------------
static dterr_t*
test_dteventlogger_when_empty(void)
{
    dterr_t* dterr = NULL;

    dteventlogger_t _eventlogger1 = { 0 }, *eventlogger1 = &_eventlogger1;

    int32_t* item = NULL;
    DTERR_C(dteventlogger_init(eventlogger1, 10, sizeof(*item)));

    DTERR_C(dteventlogger_get_item(eventlogger1, 0, (void**)&item));
    DTUNITTEST_ASSERT_PTR(item, ==, NULL);

cleanup:
    dteventlogger_dispose(eventlogger1);
    return dterr;
}

// ---------------------------------------------------------------------------------------------
static dterr_t*
test_dteventlogger_append_and_get_int32(void)
{
    dterr_t* dterr = NULL;

    dteventlogger_t logger = { 0 };
    const int32_t count = 3;
    DTERR_C(dteventlogger_init(&logger, count, sizeof(int32_t)));

    int32_t v1 = 10, v2 = 20;
    DTERR_C(dteventlogger_append(&logger, &v1));
    DTERR_C(dteventlogger_append(&logger, &v2));

    int32_t* item = NULL;
    // index 0 -> first appended (v1)
    DTERR_C(dteventlogger_get_item(&logger, 0, (void**)&item));
    DTUNITTEST_ASSERT_NOT_NULL(item);
    DTUNITTEST_ASSERT_INT(*item, ==, v1);

    // index 1 -> second appended (v2)
    DTERR_C(dteventlogger_get_item(&logger, 1, (void**)&item));
    DTUNITTEST_ASSERT_NOT_NULL(item);
    DTUNITTEST_ASSERT_INT(*item, ==, v2);

cleanup:
    dteventlogger_dispose(&logger);
    return dterr;
}

// ---------------------------------------------------------------------------------------------
static dterr_t*
test_dteventlogger_append_and_get_item1(void)
{
    dterr_t* dterr = NULL;

    dteventlogger_t logger = { 0 };
    const int32_t count = 3;
    DTERR_C(dteventlogger_init(&logger, count, sizeof(dteventlogger_item1_t)));

    dteventlogger_item1_t v1 = { .timestamp = 1000, .value1 = 10, .value2 = 20 };
    dteventlogger_item1_t v2 = { .timestamp = 2000, .value1 = 30, .value2 = 40 };
    DTERR_C(dteventlogger_append(&logger, &v1));
    DTERR_C(dteventlogger_append(&logger, &v2));

    dteventlogger_item1_t* item = NULL;
    // index 0 -> first appended (v1)
    DTERR_C(dteventlogger_get_item(&logger, 0, (void**)&item));
    DTUNITTEST_ASSERT_NOT_NULL(item);
    DTUNITTEST_ASSERT_UINT64(item->timestamp, ==, v1.timestamp);
    DTUNITTEST_ASSERT_INT(item->value1, ==, v1.value1);
    DTUNITTEST_ASSERT_INT(item->value2, ==, v1.value2);

    // index 1 -> second appended (v2)
    DTERR_C(dteventlogger_get_item(&logger, 1, (void**)&item));
    DTUNITTEST_ASSERT_NOT_NULL(item);
    DTUNITTEST_ASSERT_UINT64(item->timestamp, ==, v2.timestamp);
    DTUNITTEST_ASSERT_INT(item->value1, ==, v2.value1);
    DTUNITTEST_ASSERT_INT(item->value2, ==, v2.value2);

cleanup:
    dteventlogger_dispose(&logger);
    return dterr;
}

// ---------------------------------------------------------------------------------------------
static dterr_t*
test_dteventlogger_out_of_range(void)
{
    dterr_t* dterr = NULL;

    dteventlogger_t logger = { 0 };
    DTERR_C(dteventlogger_init(&logger, 3, sizeof(int32_t)));

    int32_t v = 42;
    DTERR_C(dteventlogger_append(&logger, &v));

    int32_t* item = (int32_t*)0x1; // non-NULL
    // index >= write_index (1) should yield NULL
    DTERR_C(dteventlogger_get_item(&logger, 1, (void**)&item));
    DTUNITTEST_ASSERT_PTR(item, ==, NULL);

    // index >= item_count - 1 (2) should also yield NULL
    DTERR_C(dteventlogger_get_item(&logger, 2, (void**)&item));
    DTUNITTEST_ASSERT_PTR(item, ==, NULL);

cleanup:
    dteventlogger_dispose(&logger);
    return dterr;
}

// ---------------------------------------------------------------------------------------------
static dterr_t*
test_dteventlogger_wrap_around1(void)
{
    dterr_t* dterr = NULL;

    dteventlogger_t logger = { 0 };
    const int32_t count = 2;
    DTERR_C(dteventlogger_init(&logger, count, sizeof(int32_t)));

    int32_t v1 = 1, v2 = 2, v3 = 3;
    DTERR_C(dteventlogger_append(&logger, &v1));
    DTERR_C(dteventlogger_append(&logger, &v2));
    DTERR_C(dteventlogger_append(&logger, &v3));

    int32_t* item = NULL;
    // After 3 appends to capacity 2, buffer holds [v3, *unusable because write pointer points here*]
    DTERR_C(dteventlogger_get_item(&logger, 0, (void**)&item));
    DTUNITTEST_ASSERT_INT(*item, ==, v3);

    DTERR_C(dteventlogger_get_item(&logger, 1, (void**)&item));
    DTUNITTEST_ASSERT_PTR(item, ==, NULL);

cleanup:
    dteventlogger_dispose(&logger);
    return dterr;
}

// ---------------------------------------------------------------------------------------------
static dterr_t*
test_dteventlogger_wrap_around2(void)
{
    dterr_t* dterr = NULL;

    dteventlogger_t logger = { 0 };
    const int32_t count = 3;
    DTERR_C(dteventlogger_init(&logger, count, sizeof(int32_t)));

    int32_t v1 = 1, v2 = 2, v3 = 3;
    DTERR_C(dteventlogger_append(&logger, &v1));
    DTERR_C(dteventlogger_append(&logger, &v2));
    DTERR_C(dteventlogger_append(&logger, &v3));

    int32_t* item = NULL;
    // After 3 appends to capacity 3, buffer holds [*unusable because write pointer points here*, v2, v3]
    DTERR_C(dteventlogger_get_item(&logger, 0, (void**)&item));
    DTUNITTEST_ASSERT_INT(*item, ==, v2);

    DTERR_C(dteventlogger_get_item(&logger, 1, (void**)&item));
    DTUNITTEST_ASSERT_INT(*item, ==, v3);

    DTERR_C(dteventlogger_get_item(&logger, 2, (void**)&item));
    DTUNITTEST_ASSERT_PTR(item, ==, NULL);

cleanup:
    dteventlogger_dispose(&logger);
    return dterr;
}

// ---------------------------------------------------------------------------------------------
static dterr_t*
test_dteventlogger_wrap_around3(void)
{
    dterr_t* dterr = NULL;

    dteventlogger_t logger = { 0 };
    dteventlogger_t clone = { 0 };
    const int32_t count = 3;
    DTERR_C(dteventlogger_init(&logger, count, sizeof(int32_t)));

    int32_t v1 = 1, v2 = 2, v3 = 3, v4 = 4;
    DTERR_C(dteventlogger_append(&logger, &v1));
    DTERR_C(dteventlogger_append(&logger, &v2));
    DTERR_C(dteventlogger_append(&logger, &v3));
    DTERR_C(dteventlogger_append(&logger, &v4));

    int32_t* item = NULL;
    // After 3 appends to capacity 3, buffer holds [v4, *unusable because write pointer points here*, v3]
    DTERR_C(dteventlogger_get_item(&logger, 0, (void**)&item));
    DTUNITTEST_ASSERT_INT(*item, ==, v3);

    DTERR_C(dteventlogger_get_item(&logger, 1, (void**)&item));
    DTUNITTEST_ASSERT_INT(*item, ==, v4);

    DTERR_C(dteventlogger_get_item(&logger, 2, (void**)&item));
    DTUNITTEST_ASSERT_PTR(item, ==, NULL);

    // clone should give same results
    DTERR_C(dteventlogger_clone(&logger, &clone));
    DTERR_C(dteventlogger_get_item(&clone, 0, (void**)&item));
    DTUNITTEST_ASSERT_INT(*item, ==, v3);
    DTERR_C(dteventlogger_get_item(&clone, 1, (void**)&item));
    DTUNITTEST_ASSERT_INT(*item, ==, v4);
    DTERR_C(dteventlogger_get_item(&clone, 2, (void**)&item));
    DTUNITTEST_ASSERT_PTR(item, ==, NULL);

cleanup:
    dteventlogger_dispose(&clone);
    dteventlogger_dispose(&logger);
    return dterr;
}

// ---------------------------------------------------------------------------------------------
static dterr_t*
test_dteventlogger_clone(void)
{
    dterr_t* dterr = NULL;

    dteventlogger_t original = { 0 };
    const int32_t count = 3;
    DTERR_C(dteventlogger_init(&original, count, sizeof(int32_t)));

    int32_t a = 7, b = 8;
    DTERR_C(dteventlogger_append(&original, &a));
    DTERR_C(dteventlogger_append(&original, &b));

    // Clone
    dteventlogger_t clone = { 0 };
    DTERR_C(dteventlogger_clone(&original, &clone));

    int32_t* item = NULL;
    // Clone should have same items
    DTERR_C(dteventlogger_get_item(&clone, 0, (void**)&item));
    DTUNITTEST_ASSERT_INT(*item, ==, a);

    DTERR_C(dteventlogger_get_item(&clone, 1, (void**)&item));
    DTUNITTEST_ASSERT_INT(*item, ==, b);

    // Modify original and ensure clone unaffected
    int32_t c = 9;
    DTERR_C(dteventlogger_append(&original, &c));

    DTERR_C(dteventlogger_get_item(&clone, 2, (void**)&item));
    DTUNITTEST_ASSERT_PTR(item, ==, NULL);

cleanup:
    dteventlogger_dispose(&original);
    dteventlogger_dispose(&clone);
    return dterr;
}

// ---------------------------------------------------------------------------------------------
static dterr_t*
test_dteventlogger_str(void)
{
    dterr_t* dterr = NULL;

    dteventlogger_t logger = { 0 };
    const int32_t count = 5;
    DTERR_C(dteventlogger_init(&logger, count, sizeof(dteventlogger_item1_t)));

    dteventlogger_item1_t v1 = { .timestamp = 1000, .value1 = 10, .value2 = 20 };
    dteventlogger_item1_t v2 = { .timestamp = 2000, .value1 = 30, .value2 = 40 };
    dteventlogger_item1_t v3 = { .timestamp = 3000, .value1 = 50, .value2 = 60 };
    DTERR_C(dteventlogger_append(&logger, &v1));
    DTERR_C(dteventlogger_append(&logger, &v2));
    DTERR_C(dteventlogger_append(&logger, &v3));

    char* str = NULL;
    DTERR_C(dteventlogger_printf_item1(&logger, "TestLogger", "Value1", NULL, &str));
    dtlog_info(TAG, "\n%s", str);

cleanup:
    dtstr_dispose(str);
    dteventlogger_dispose(&logger);
    return dterr;
}
// ---------------------------------------------------------------------------------------------
// suite runner
void
test_dteventlogger(DTUNITTEST_SUITE_ARGS)
{
    DTUNITTEST_RUN_TEST(test_dteventlogger_when_empty);
    DTUNITTEST_RUN_TEST(test_dteventlogger_append_and_get_int32);
    DTUNITTEST_RUN_TEST(test_dteventlogger_append_and_get_item1);
    DTUNITTEST_RUN_TEST(test_dteventlogger_out_of_range);
    DTUNITTEST_RUN_TEST(test_dteventlogger_wrap_around1);
    DTUNITTEST_RUN_TEST(test_dteventlogger_wrap_around2);
    DTUNITTEST_RUN_TEST(test_dteventlogger_wrap_around3);
    DTUNITTEST_RUN_TEST(test_dteventlogger_clone);
    DTUNITTEST_RUN_TEST(test_dteventlogger_str);
}
