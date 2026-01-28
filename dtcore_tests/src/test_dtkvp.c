#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dtbuffer.h>
#include <dtcore/dterr.h>
#include <dtcore/dtkvp.h>

#include <dtcore/dtpackx.h>
#include <dtcore/dtunittest.h>
#include <dtcore_tests.h>

#define TAG "test_dtcore_dtkvp"

// ============================================================================
// Example: minimal useful roundtrip with 2 key-value pairs
// ============================================================================
static dterr_t*
test_dtcore_dtkvp_example_roundtrip(void)
{
    dterr_t* dterr = NULL;
    dtkvp_list_t list;
    dtkvp_list_t list2;
    uint8_t* buf = NULL;

    DTERR_C(dtkvp_list_init(&list));

    DTERR_C(dtkvp_list_set(&list, "a", "1"));
    DTERR_C(dtkvp_list_set(&list, "b", "2"));

    int32_t need = 0;
    DTERR_C(dtkvp_list_packx_length(&list, &need));

    buf = (uint8_t*)calloc((size_t)need, 1);
    DTUNITTEST_ASSERT_NOT_NULL(buf);

    int32_t p = 0;
    DTERR_C(dtkvp_list_packx(&list, buf, &p, need));
    DTUNITTEST_ASSERT_INT(p, ==, need);

    DTERR_C(dtkvp_list_init(&list2));

    int32_t p2 = 0;
    DTERR_C(dtkvp_list_unpackx(&list2, buf, &p2, need));
    DTUNITTEST_ASSERT_INT(p2, ==, need);

    const char* val = NULL;
    DTERR_C(dtkvp_list_get(&list2, "a", &val));
    DTUNITTEST_ASSERT_EQUAL_STRING(val, "1");

    DTERR_C(dtkvp_list_get(&list2, "b", &val));
    DTUNITTEST_ASSERT_EQUAL_STRING(val, "2");

cleanup:

    dtkvp_list_dispose(&list2);
    dtkvp_list_dispose(&list);
    free(buf);

    return dterr;
}

// ============================================================================
// init: bad args
// ============================================================================
static dterr_t*
test_dtcore_dtkvp_01_init_bad_args(void)
{
    dterr_t* dterr = NULL;

    dterr_t* err = dtkvp_list_init(NULL);
    DTUNITTEST_ASSERT_DTERR(err, DTERR_ARGUMENT_NULL);

cleanup:
    return dterr;
}

// ============================================================================
// init OK
// ============================================================================
static dterr_t*
test_dtcore_dtkvp_02_init_ok(void)
{
    dterr_t* dterr = NULL;
    dtkvp_list_t list;

    DTERR_C(dtkvp_list_init(&list));

    DTUNITTEST_ASSERT_NOT_NULL(list.storage);
    DTUNITTEST_ASSERT_NOT_NULL(list.items);
    DTUNITTEST_ASSERT_INT(list.count, ==, 32);

cleanup:
    dtkvp_list_dispose(&list);
    return dterr;
}

// ============================================================================
// set / get / overwrite / remove
// ============================================================================
static dterr_t*
test_dtcore_dtkvp_03_set_get_overwrite_remove(void)
{
    dterr_t* dterr = NULL;
    dtkvp_list_t list;
    const char* val = NULL;

    DTERR_C(dtkvp_list_init(&list));

    DTERR_C(dtkvp_list_set(&list, "x", "100"));
    DTERR_C(dtkvp_list_get(&list, "x", &val));
    DTUNITTEST_ASSERT_EQUAL_STRING(val, "100");

    // overwrite
    DTERR_C(dtkvp_list_set(&list, "x", "200"));
    DTERR_C(dtkvp_list_get(&list, "x", &val));
    DTUNITTEST_ASSERT_EQUAL_STRING(val, "200");

    // remove
    DTERR_C(dtkvp_list_set(&list, "x", NULL));
    DTERR_C(dtkvp_list_get(&list, "x", &val));
    DTUNITTEST_ASSERT_NULL(val);

cleanup:
    dtkvp_list_dispose(&list);
    return dterr;
}

// ============================================================================
// growth (force resize)
// ============================================================================
static dterr_t*
test_dtcore_dtkvp_04_growth(void)
{
    dterr_t* dterr = NULL;
    dtkvp_list_t list;

    DTERR_C(dtkvp_list_init(&list));

    char key[32];
    char val[32];

    for (int i = 0; i < 40; i++)
    {
        snprintf(key, sizeof(key), "k%d", i);
        snprintf(val, sizeof(val), "v%d", i);
        DTERR_C(dtkvp_list_set(&list, key, val));
    }

    // must have grown past 32
    DTUNITTEST_ASSERT_TRUE(list.count >= 40);

    const char* out = NULL;
    DTERR_C(dtkvp_list_get(&list, "k39", &out));
    DTUNITTEST_ASSERT_EQUAL_STRING(out, "v39");

cleanup:
    dtkvp_list_dispose(&list);
    return dterr;
}

// ============================================================================
// packx_length matches packx output size
// ============================================================================
static dterr_t*
test_dtcore_dtkvp_05_packx_length_matches(void)
{
    dterr_t* dterr = NULL;
    dtkvp_list_t list;
    uint8_t* buf = NULL;

    DTERR_C(dtkvp_list_init(&list));

    DTERR_C(dtkvp_list_set(&list, "a", "hello"));
    DTERR_C(dtkvp_list_set(&list, "b", "world"));

    int32_t L = 0;
    DTERR_C(dtkvp_list_packx_length(&list, &L));

    buf = (uint8_t*)calloc((size_t)L, 1);
    DTUNITTEST_ASSERT_NOT_NULL(buf);

    int32_t off = 0;
    DTERR_C(dtkvp_list_packx(&list, buf, &off, L));
    DTUNITTEST_ASSERT_INT(off, ==, L);

cleanup:
    dtkvp_list_dispose(&list);
    free(buf);
    return dterr;
}

// ============================================================================
// packx respecting offset
// ============================================================================
static dterr_t*
test_dtcore_dtkvp_06_packx_offset(void)
{
    dterr_t* dterr = NULL;
    dtkvp_list_t list;
    uint8_t* buf = NULL;

    DTERR_C(dtkvp_list_init(&list));
    DTERR_C(dtkvp_list_set(&list, "x", "7"));

    int32_t need = 0;
    DTERR_C(dtkvp_list_packx_length(&list, &need));

    const int32_t off = 5;
    buf = (uint8_t*)calloc((size_t)(off + need), 1);
    DTUNITTEST_ASSERT_NOT_NULL(buf);

    int32_t p = off;
    DTERR_C(dtkvp_list_packx(&list, buf, &p, off + need));
    DTUNITTEST_ASSERT_INT(p, ==, off + need);

cleanup:
    dtkvp_list_dispose(&list);
    free(buf);
    return dterr;
}

// ============================================================================
// unpackx into empty list
// ============================================================================
static dterr_t*
test_dtcore_dtkvp_07_unpack_into_empty(void)
{
    dterr_t* dterr = NULL;
    dtkvp_list_t src;
    dtkvp_list_t dst;
    uint8_t* buf = NULL;

    DTERR_C(dtkvp_list_init(&src));
    DTERR_C(dtkvp_list_set(&src, "a", "1"));
    DTERR_C(dtkvp_list_set(&src, "b", "2"));

    int32_t need = 0;
    DTERR_C(dtkvp_list_packx_length(&src, &need));

    buf = (uint8_t*)calloc((size_t)need, 1);
    DTUNITTEST_ASSERT_NOT_NULL(buf);

    int32_t p = 0;
    DTERR_C(dtkvp_list_packx(&src, buf, &p, need));

    DTERR_C(dtkvp_list_init(&dst));

    int32_t p2 = 0;
    DTERR_C(dtkvp_list_unpackx(&dst, buf, &p2, need));

    const char* v = NULL;
    DTERR_C(dtkvp_list_get(&dst, "a", &v));
    DTUNITTEST_ASSERT_EQUAL_STRING(v, "1");

    DTERR_C(dtkvp_list_get(&dst, "b", &v));
    DTUNITTEST_ASSERT_EQUAL_STRING(v, "2");

cleanup:
    dtkvp_list_dispose(&dst);
    dtkvp_list_dispose(&src);
    free(buf);
    return dterr;
}

// ============================================================================
// unpackx merges + overwrites existing keys
// ============================================================================
static dterr_t*
test_dtcore_dtkvp_08_unpack_merge_overwrite(void)
{
    dterr_t* dterr = NULL;
    dtkvp_list_t src;
    dtkvp_list_t dst;
    uint8_t* buf = NULL;

    DTERR_C(dtkvp_list_init(&src));
    DTERR_C(dtkvp_list_set(&src, "a", "100"));
    DTERR_C(dtkvp_list_set(&src, "c", "300"));

    int32_t need = 0;
    DTERR_C(dtkvp_list_packx_length(&src, &need));

    buf = (uint8_t*)calloc((size_t)need, 1);
    DTUNITTEST_ASSERT_NOT_NULL(buf);

    int32_t p = 0;
    DTERR_C(dtkvp_list_packx(&src, buf, &p, need));

    DTERR_C(dtkvp_list_init(&dst));
    DTERR_C(dtkvp_list_set(&dst, "a", "orig"));
    DTERR_C(dtkvp_list_set(&dst, "b", "200"));

    int32_t p2 = 0;
    DTERR_C(dtkvp_list_unpackx(&dst, buf, &p2, need));

    const char* v = NULL;

    DTERR_C(dtkvp_list_get(&dst, "a", &v));
    DTUNITTEST_ASSERT_EQUAL_STRING(v, "100");

    DTERR_C(dtkvp_list_get(&dst, "b", &v));
    DTUNITTEST_ASSERT_EQUAL_STRING(v, "200");

    DTERR_C(dtkvp_list_get(&dst, "c", &v));
    DTUNITTEST_ASSERT_EQUAL_STRING(v, "300");

cleanup:
    dtkvp_list_dispose(&dst);
    dtkvp_list_dispose(&src);
    free(buf);
    return dterr;
}

// ============================================================================
// unpackx: insufficient buffer → DTERR_TRUNCATED
// ============================================================================
static dterr_t*
test_dtcore_dtkvp_09_unpack_insufficient_buffer(void)
{
    dterr_t* dterr = NULL;
    dtkvp_list_t src;
    dtkvp_list_t dst;
    uint8_t* buf = NULL;

    DTERR_C(dtkvp_list_init(&src));
    DTERR_C(dtkvp_list_set(&src, "a", "1"));

    int32_t need = 0;
    DTERR_C(dtkvp_list_packx_length(&src, &need));

    buf = (uint8_t*)calloc((size_t)need, 1);
    DTUNITTEST_ASSERT_NOT_NULL(buf);

    int32_t p = 0;
    DTERR_C(dtkvp_list_packx(&src, buf, &p, need));

    DTERR_C(dtkvp_list_init(&dst));

    int32_t p2 = 0;
    dterr = dtkvp_list_unpackx(&dst, buf, &p2, need - 1);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_OVERFLOW);

cleanup:
    dtkvp_list_dispose(&dst);
    dtkvp_list_dispose(&src);
    free(buf);
    return dterr;
}

// ============================================================================
// suite entry point
// ============================================================================
void
test_dtcore_dtkvp(DTUNITTEST_SUITE_ARGS)
{

    DTUNITTEST_RUN_TEST(test_dtcore_dtkvp_example_roundtrip);

    DTUNITTEST_RUN_TEST(test_dtcore_dtkvp_01_init_bad_args);
    DTUNITTEST_RUN_TEST(test_dtcore_dtkvp_02_init_ok);
    DTUNITTEST_RUN_TEST(test_dtcore_dtkvp_03_set_get_overwrite_remove);
    DTUNITTEST_RUN_TEST(test_dtcore_dtkvp_04_growth);

    DTUNITTEST_RUN_TEST(test_dtcore_dtkvp_05_packx_length_matches);
    DTUNITTEST_RUN_TEST(test_dtcore_dtkvp_06_packx_offset);
    DTUNITTEST_RUN_TEST(test_dtcore_dtkvp_07_unpack_into_empty);
    DTUNITTEST_RUN_TEST(test_dtcore_dtkvp_08_unpack_merge_overwrite);
    DTUNITTEST_RUN_TEST(test_dtcore_dtkvp_09_unpack_insufficient_buffer);
}
