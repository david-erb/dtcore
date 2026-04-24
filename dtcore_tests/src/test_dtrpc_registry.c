#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dterr.h>
#include <dtcore/dtheaper.h>
#include <dtcore/dtkvp.h>
#include <dtcore/dtlog.h>
#include <dtcore/dtrpc.h>
#include <dtcore/dtrpc_registry.h>
#include <dtcore/dtstr.h>
#include <dtcore/dtunittest.h>
#include <dtcore/dtvtable.h>

#include <dtcore_tests.h>

#define TAG "test_dtcore_dtrpc_registry"

// ------------------------------------------------------------------------
// Example: Small, readable "happy path" showing basic registry and retrieval.
static dterr_t*
test_dtcore_dtrpc_registry_example_basic(void)
{
    dterr_t* dterr = NULL;
    dtrpc_handle rpc_handle1 = (dtrpc_handle)0x01;
    dtrpc_handle rpc_handle2 = (dtrpc_handle)0x02;
    dtrpc_registry_t* registry = NULL;
    dtrpc_handle retrieved_handle = NULL;

    DTERR_C(dtrpc_registry_create(&registry));
    DTERR_C(dtrpc_registry_add(registry, rpc_handle1));
    DTERR_C(dtrpc_registry_add(registry, rpc_handle2));

    DTERR_C(dtrpc_registry_get(registry, 0, &retrieved_handle));
    DTUNITTEST_ASSERT_TRUE(retrieved_handle == rpc_handle1);

    DTERR_C(dtrpc_registry_get(registry, 1, &retrieved_handle));
    DTUNITTEST_ASSERT_TRUE(retrieved_handle == rpc_handle2);

cleanup:
    dtrpc_registry_dispose(registry);
    return dterr;
}

// ------------------------------------------------------------------------
// newly created registry should return NULL for any lookup.
static dterr_t*
test_dtcore_dtrpc_registry_get_from_empty_returns_null(void)
{
    dterr_t* dterr = NULL;
    dtrpc_registry_t* registry = NULL;
    dtrpc_handle retrieved_handle = (dtrpc_handle)0x1234;

    DTERR_C(dtrpc_registry_create(&registry));

    DTERR_C(dtrpc_registry_get(registry, 0, &retrieved_handle));
    DTUNITTEST_ASSERT_TRUE(retrieved_handle == NULL);

    DTERR_C(dtrpc_registry_get(registry, 99, &retrieved_handle));
    DTUNITTEST_ASSERT_TRUE(retrieved_handle == NULL);

cleanup:
    dtrpc_registry_dispose(registry);
    return dterr;
}

// ------------------------------------------------------------------------
// negative and one-past-end indexes should safely return NULL.
static dterr_t*
test_dtcore_dtrpc_registry_get_invalid_indexes_return_null(void)
{
    dterr_t* dterr = NULL;
    dtrpc_registry_t* registry = NULL;
    dtrpc_handle rpc_handle1 = (dtrpc_handle)0x11;
    dtrpc_handle rpc_handle2 = (dtrpc_handle)0x22;
    dtrpc_handle retrieved_handle = (dtrpc_handle)0x9999;

    DTERR_C(dtrpc_registry_create(&registry));
    DTERR_C(dtrpc_registry_add(registry, rpc_handle1));
    DTERR_C(dtrpc_registry_add(registry, rpc_handle2));

    DTERR_C(dtrpc_registry_get(registry, -1, &retrieved_handle));
    DTUNITTEST_ASSERT_TRUE(retrieved_handle == NULL);

    DTERR_C(dtrpc_registry_get(registry, 2, &retrieved_handle));
    DTUNITTEST_ASSERT_TRUE(retrieved_handle == NULL);

    DTERR_C(dtrpc_registry_get(registry, 1000, &retrieved_handle));
    DTUNITTEST_ASSERT_TRUE(retrieved_handle == NULL);

cleanup:
    dtrpc_registry_dispose(registry);
    return dterr;
}

// ------------------------------------------------------------------------
// adding more than initial capacity should grow the registry.
static dterr_t*
test_dtcore_dtrpc_registry_add_grows_capacity(void)
{
    dterr_t* dterr = NULL;
    dtrpc_registry_t* registry = NULL;
    int32_t i = 0;
    int32_t add_count = DTRPC_REGISTRY_GROWTH + 5;

    DTERR_C(dtrpc_registry_create(&registry));
    DTUNITTEST_ASSERT_TRUE(registry->count == 0);
    DTUNITTEST_ASSERT_TRUE(registry->capacity == DTRPC_REGISTRY_INITIAL_CAPACITY);

    for (i = 0; i < add_count; i++)
    {
        DTERR_C(dtrpc_registry_add(registry, (dtrpc_handle)(size_t)(i + 1)));
    }

    DTUNITTEST_ASSERT_TRUE(registry->count == add_count);
    DTUNITTEST_ASSERT_TRUE(registry->capacity == DTRPC_REGISTRY_INITIAL_CAPACITY + DTRPC_REGISTRY_GROWTH);

cleanup:
    dtrpc_registry_dispose(registry);
    return dterr;
}

// ------------------------------------------------------------------------
// verify all entries survive a growth and preserve insertion order.
static dterr_t*
test_dtcore_dtrpc_registry_growth_preserves_order(void)
{
    dterr_t* dterr = NULL;
    dtrpc_registry_t* registry = NULL;
    dtrpc_handle retrieved_handle = NULL;
    int32_t i = 0;
    int32_t add_count = 2 * DTRPC_REGISTRY_GROWTH + 5;

    DTERR_C(dtrpc_registry_create(&registry));

    for (i = 0; i < add_count; i++)
    {
        DTERR_C(dtrpc_registry_add(registry, (dtrpc_handle)(size_t)(0x100 + i)));
    }

    DTUNITTEST_ASSERT_TRUE(registry->count == add_count);
    DTUNITTEST_ASSERT_TRUE(registry->capacity == DTRPC_REGISTRY_INITIAL_CAPACITY + 2 * DTRPC_REGISTRY_GROWTH);

    for (i = 0; i < add_count; i++)
    {
        DTERR_C(dtrpc_registry_get(registry, i, &retrieved_handle));
        DTUNITTEST_ASSERT_TRUE(retrieved_handle == (dtrpc_handle)(size_t)(0x100 + i));
    }

cleanup:
    dtrpc_registry_dispose(registry);
    return dterr;
}

// ------------------------------------------------------------------------
void
test_dtcore_dtrpc_registry(DTUNITTEST_SUITE_ARGS)
{
    // Examples first.
    DTUNITTEST_RUN_TEST(test_dtcore_dtrpc_registry_example_basic);

    // Additional coverage.
    DTUNITTEST_RUN_TEST(test_dtcore_dtrpc_registry_get_from_empty_returns_null);
    DTUNITTEST_RUN_TEST(test_dtcore_dtrpc_registry_get_invalid_indexes_return_null);
    DTUNITTEST_RUN_TEST(test_dtcore_dtrpc_registry_add_grows_capacity);
    DTUNITTEST_RUN_TEST(test_dtcore_dtrpc_registry_growth_preserves_order);
}