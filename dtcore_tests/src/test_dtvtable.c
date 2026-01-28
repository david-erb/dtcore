#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include <dtcore/dterr.h>
#include <dtcore/dtunittest.h>
#include <dtcore/dtvtable.h>
#include <dtcore_tests.h>

/* Small helper: create a stack-backed registry over caller-provided arrays. */
static inline dtvtable_registry_t
make_registry(int32_t* numbers, void** tables, int capacity)
{
    dtvtable_registry_t reg;
    reg.model_numbers = numbers;
    reg.vtables = tables;
    reg.max_vtables = capacity;
    return reg;
}

/* A couple of dummy vtables to differentiate pointer identity in tests. */
typedef struct
{
    int dummy;
} demo_vt_t;
static demo_vt_t VT_A = { 1 };
static demo_vt_t VT_B = { 2 };

/* ------------------------------------------------------------------------------------------------ */
/* Example tests: short, readable demonstrations of typical usage patterns. */

/* Basic publish and fetch. */
dterr_t*
test_dtcore_dtvtable_example_basic_publish_and_get(void)
{
    dterr_t* dterr = NULL;
    int32_t numbers[4] = { 0 };
    void* tables[4] = { 0 };
    dtvtable_registry_t reg = make_registry(numbers, tables, 4);

    dterr = dtvtable_set(&reg, 1001, &VT_A);
    DTUNITTEST_ASSERT_NULL(dterr);

    void* out = NULL;
    dterr = dtvtable_get(&reg, 1001, &out);
    DTUNITTEST_ASSERT_NULL(dterr);
    DTUNITTEST_ASSERT_PTR(out, ==, &VT_A);
cleanup:
    return dterr;
}
// -------------------------------------------------------------------------------------------

/* Idempotent publish: publishing the same (model, pointer) again is success and leaves the mapping unchanged. */
dterr_t*
test_dtcore_dtvtable_example_idempotent_publish(void)
{
    dterr_t* dterr = NULL;
    int32_t numbers[2] = { 0 };
    void* tables[2] = { 0 };
    dtvtable_registry_t reg = make_registry(numbers, tables, 2);

    dterr = dtvtable_set(&reg, 2002, &VT_A);
    DTUNITTEST_ASSERT_NULL(dterr);

    /* Publish again with the same pointer — considered success by contract. */
    dterr = dtvtable_set(&reg, 2002, &VT_A);
    DTUNITTEST_ASSERT_NULL(dterr);

    void* out = NULL;
    dterr = dtvtable_get(&reg, 2002, &out);
    DTUNITTEST_ASSERT_NULL(dterr);
    DTUNITTEST_ASSERT_PTR(out, ==, &VT_A);
cleanup:
    return dterr;
}
// -------------------------------------------------------------------------------------------

/* Re-publish with a different pointer: per contract, considered success and must not change the existing mapping. */
dterr_t*
test_dtcore_dtvtable_example_existing_model_is_success_and_unchanged(void)
{
    dterr_t* dterr = NULL;
    int32_t numbers[3] = { 0 };
    void* tables[3] = { 0 };
    dtvtable_registry_t reg = make_registry(numbers, tables, 3);

    dterr = dtvtable_set(&reg, 3003, &VT_A);
    DTUNITTEST_ASSERT_NULL(dterr);

    /* Try to publish a different pointer to the same model. Contract: success, no remap. */
    dterr = dtvtable_set(&reg, 3003, &VT_B);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_EXISTS);

    void* out = NULL;
    dterr = dtvtable_get(&reg, 3003, &out);
    DTUNITTEST_ASSERT_NULL(dterr);
    DTUNITTEST_ASSERT_PTR(out, ==, &VT_A);
cleanup:
    return dterr;
}
// -------------------------------------------------------------------------------------------

/* Compact-layout gotcha: get() stops at first empty slot; a "hole" before a later entry breaks lookups. */
dterr_t*
test_dtcore_dtvtable_example_compact_layout_gotcha(void)
{
    dterr_t* dterr = NULL;
    int32_t numbers[4] = { 0, 0, 4004, 0 }; /* hole before a used slot */
    void* tables[4] = { 0, 0, &VT_A, 0 };
    dtvtable_registry_t reg = make_registry(numbers, tables, 4);

    void* out = NULL;
    dterr = dtvtable_get(&reg, 4004, &out);
    DTUNITTEST_ASSERT_NULL(dterr);
    DTUNITTEST_ASSERT_PTR(out, ==, &VT_A);
cleanup:
    return dterr;
}
// -------------------------------------------------------------------------------------------

/* ------------------------------------------------------------------------------------------------ */
/* Systematic tests (numbered): edge cases, error paths, and boundary conditions. */

/* 01: Lookup missing model returns NOTFOUND. */
dterr_t*
test_dtcore_dtvtable_01_lookup_missing_returns_notfound(void)
{
    dterr_t* dterr = NULL;
    int32_t numbers[2] = { 0 };
    void* tables[2] = { 0 };
    dtvtable_registry_t reg = make_registry(numbers, tables, 2);

    void* out = (void*)0xDEADBEEF;
    dterr = dtvtable_get(&reg, 9999, &out);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_NOTFOUND);
    DTUNITTEST_ASSERT_PTR(out, ==, (void*)0xDEADBEEF);
cleanup:
    return dterr;
}
// -------------------------------------------------------------------------------------------

/* 02: Full registry: set() fails cleanly. */
dterr_t*
test_dtcore_dtvtable_02_publish_fails_when_full(void)
{
    dterr_t* dterr = NULL;
    int32_t numbers[2] = { 1, 2 }; /* non-zero means occupied */
    void* tables[2] = { &VT_A, &VT_B };
    dtvtable_registry_t reg = make_registry(numbers, tables, 2);

    dterr = dtvtable_set(&reg, 7777, &VT_A);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_FAIL);
cleanup:
    return dterr;
}
// -------------------------------------------------------------------------------------------

/* 03: BADARG: null registry pointer on set/get. */
dterr_t*
test_dtcore_dtvtable_03_badarg_null_registry_pointer(void)
{
    dterr_t* dterr = NULL;
    void* out = NULL;

    dterr = dtvtable_set(NULL, 1, &VT_A);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_ARGUMENT_NULL);

    dterr = dtvtable_get(NULL, 1, &out);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_ARGUMENT_NULL);
cleanup:
    return dterr;
}
// -------------------------------------------------------------------------------------------

/* 04: BADARG: invalid registry arrays or capacity. */
dterr_t*
test_dtcore_dtvtable_04_badarg_invalid_registry_fields(void)
{
    dterr_t* dterr = NULL;
    int32_t numbers[1] = { 0 };
    void* tables[1] = { 0 };

    /* Null arrays */
    dtvtable_registry_t reg_null_arrays = { 0, 0, 1 };
    dterr = dtvtable_set(&reg_null_arrays, 42, &VT_A);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_ARGUMENT_NULL);
    dterr = dtvtable_get(&reg_null_arrays, 42, NULL);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_ARGUMENT_NULL);

    /* Non-positive capacity */
    dtvtable_registry_t reg_zero_cap = { numbers, tables, 0 };
    dterr = dtvtable_set(&reg_zero_cap, 42, &VT_A);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_BADARG);
    dterr = dtvtable_get(&reg_zero_cap, 42, NULL);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_BADARG);
cleanup:
    return dterr;
}
// -------------------------------------------------------------------------------------------

/* 05: BADARG: model_number == 0 is not allowed. */
dterr_t*
test_dtcore_dtvtable_05_badarg_zero_model_number(void)
{
    dterr_t* dterr = NULL;
    int32_t numbers[2] = { 0 };
    void* tables[2] = { 0 };
    dtvtable_registry_t reg = make_registry(numbers, tables, 2);

    dterr = dtvtable_set(&reg, 0, &VT_A);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_BADARG);

    dterr = dtvtable_get(&reg, 0, NULL);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_BADARG);
cleanup:
    return dterr;
}
// -------------------------------------------------------------------------------------------

/* 06: BADARG: set() with NULL vtable pointer. */
dterr_t*
test_dtcore_dtvtable_06_badarg_null_vtable_pointer(void)
{
    dterr_t* dterr = NULL;
    int32_t numbers[2] = { 0 };
    void* tables[2] = { 0 };
    dtvtable_registry_t reg = make_registry(numbers, tables, 2);

    dterr = dtvtable_set(&reg, 55, NULL);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_BADARG);
cleanup:
    return dterr;
}
// -------------------------------------------------------------------------------------------

/* 07: Verify that a successful publish lands in the first empty slot, and subsequent publish uses the next. */
dterr_t*
test_dtcore_dtvtable_07_publish_uses_first_empty_slot(void)
{
    dterr_t* dterr = NULL;
    int32_t numbers[4] = { 0 };
    void* tables[4] = { 0 };
    dtvtable_registry_t reg = make_registry(numbers, tables, 4);

    dterr = dtvtable_set(&reg, 11, &VT_A);
    DTUNITTEST_ASSERT_NULL(dterr);
    DTUNITTEST_ASSERT_TRUE(numbers[0] == 11 && tables[0] == &VT_A);
    dterr = dtvtable_set(&reg, 22, &VT_B);
    DTUNITTEST_ASSERT_NULL(dterr);
    DTUNITTEST_ASSERT_TRUE(numbers[1] == 22 && tables[1] == &VT_B);
cleanup:
    return dterr;
}
// -------------------------------------------------------------------------------------------

/* 08: get() with NULL out pointer still reports success (and does not dereference). */
dterr_t*
test_dtcore_dtvtable_08_get_with_null_out_parameter(void)
{
    int32_t numbers[2] = { 0 };
    void* tables[2] = { 0 };
    dtvtable_registry_t reg = make_registry(numbers, tables, 2);

    dterr_t* dterr = dtvtable_set(&reg, 101, &VT_A);
    DTUNITTEST_ASSERT_NULL(dterr);

    dterr = dtvtable_get(&reg, 101, NULL);
    DTUNITTEST_ASSERT_NULL(dterr);
cleanup:
    return dterr;
}
// -------------------------------------------------------------------------------------------

/* 09: Publish multiple and retrieve each; ensure no cross-talk. */
dterr_t*
test_dtcore_dtvtable_09_publish_and_fetch_multiple(void)
{
    dterr_t* dterr = NULL;
    int32_t numbers[6] = { 0 };
    void* tables[6] = { 0 };
    dtvtable_registry_t reg = make_registry(numbers, tables, 6);

    struct
    {
        int32_t model;
        void* vt;
    } pairs[] = { { 1, &VT_A }, { 2, &VT_B }, { 3, &VT_A }, { 4, &VT_B } };

    for (size_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); ++i)
    {
        dterr = dtvtable_set(&reg, pairs[i].model, pairs[i].vt);
        DTUNITTEST_ASSERT_NULL(dterr);
    }

    for (size_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); ++i)
    {
        void* out = NULL;
        dterr = dtvtable_get(&reg, pairs[i].model, &out);
        DTUNITTEST_ASSERT_NULL(dterr);
        DTUNITTEST_ASSERT_PTR(out, ==, pairs[i].vt);
    }
cleanup:
    return dterr;
}
// -------------------------------------------------------------------------------------------

/* 10: Re-publish same model with different pointer leaves mapping unchanged (contract). */
dterr_t*
test_dtcore_dtvtable_10_republish_different_pointer_does_not_change_mapping(void)
{
    dterr_t* dterr = NULL;
    int32_t numbers[3] = { 0 };
    void* tables[3] = { 0 };
    dtvtable_registry_t reg = make_registry(numbers, tables, 3);

    dterr = dtvtable_set(&reg, 5150, &VT_A);
    DTUNITTEST_ASSERT_NULL(dterr);
    dterr = dtvtable_set(&reg, 5150, &VT_B);
    DTUNITTEST_ASSERT_DTERR(dterr, DTERR_EXISTS);

    void* out = NULL;
    dterr = dtvtable_get(&reg, 5150, &out);
    DTUNITTEST_ASSERT_NULL(dterr);
    DTUNITTEST_ASSERT_PTR(out, ==, &VT_A);
cleanup:
    return dterr;
}

// -------------------------------------------------------------------------------------------
void
test_dtcore_dtvtable(DTUNITTEST_SUITE_ARGS)
{
    DTUNITTEST_RUN_TEST(test_dtcore_dtvtable_example_basic_publish_and_get);
    DTUNITTEST_RUN_TEST(test_dtcore_dtvtable_example_idempotent_publish);
    DTUNITTEST_RUN_TEST(test_dtcore_dtvtable_example_existing_model_is_success_and_unchanged);
    DTUNITTEST_RUN_TEST(test_dtcore_dtvtable_example_compact_layout_gotcha);

    DTUNITTEST_RUN_TEST(test_dtcore_dtvtable_01_lookup_missing_returns_notfound);
    DTUNITTEST_RUN_TEST(test_dtcore_dtvtable_02_publish_fails_when_full);
    DTUNITTEST_RUN_TEST(test_dtcore_dtvtable_03_badarg_null_registry_pointer);
    DTUNITTEST_RUN_TEST(test_dtcore_dtvtable_04_badarg_invalid_registry_fields);
    DTUNITTEST_RUN_TEST(test_dtcore_dtvtable_05_badarg_zero_model_number);
    DTUNITTEST_RUN_TEST(test_dtcore_dtvtable_06_badarg_null_vtable_pointer);
    DTUNITTEST_RUN_TEST(test_dtcore_dtvtable_07_publish_uses_first_empty_slot);
    DTUNITTEST_RUN_TEST(test_dtcore_dtvtable_08_get_with_null_out_parameter);
    DTUNITTEST_RUN_TEST(test_dtcore_dtvtable_09_publish_and_fetch_multiple);
    DTUNITTEST_RUN_TEST(test_dtcore_dtvtable_10_republish_different_pointer_does_not_change_mapping);
}
