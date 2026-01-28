#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dterr.h>
#include <dtcore/dtflipper.h>
#include <dtcore/dtunittest.h>

// --------------------------------------------------------------------------------------------
static dterr_t*
test_dtflipper_init_and_control_release(void)
{
    dterr_t* dterr = NULL;

    int upper[1] = { 0 };
    int lower[1] = { 0 };
    dtflipper_t f;
    void* p = NULL;

    dterr = dtflipper_init(&f, upper, lower);
    DTUNITTEST_ASSERT_PTR(dterr, ==, NULL);

    // Control upper
    dterr = dtflipper_control_upper(&f, &p);
    DTUNITTEST_ASSERT_PTR(dterr, ==, NULL);
    DTUNITTEST_ASSERT_PTR(p, ==, upper);

    // Release upper
    dterr = dtflipper_release_upper(&f);
    DTUNITTEST_ASSERT_PTR(dterr, ==, NULL);

    // Control upper after release
    dterr = dtflipper_control_upper(&f, &p);
    DTUNITTEST_ASSERT_PTR(dterr, ==, NULL);
    DTUNITTEST_ASSERT_PTR(p, ==, upper);

    // Control lower
    dterr = dtflipper_control_lower(&f, &p);
    DTUNITTEST_ASSERT_PTR(dterr, ==, NULL);
    DTUNITTEST_ASSERT_PTR(p, ==, lower);

    // Release lower
    dterr = dtflipper_release_lower(&f);
    DTUNITTEST_ASSERT_PTR(dterr, ==, NULL);

    // Control lower after release
    dterr = dtflipper_control_lower(&f, &p);
    DTUNITTEST_ASSERT_PTR(dterr, ==, NULL);
    DTUNITTEST_ASSERT_PTR(p, ==, lower);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
static dterr_t*
test_dtflipper_flip_success(void)
{
    dterr_t* dterr = NULL;

    int upper[1] = { 1 };
    int lower[1] = { 2 };
    dtflipper_t f;

    dterr = dtflipper_init(&f, upper, lower);
    DTUNITTEST_ASSERT_PTR(dterr, ==, NULL);

    dterr = dtflipper_flip(&f);
    DTUNITTEST_ASSERT_PTR(dterr, ==, NULL);
    DTUNITTEST_ASSERT_PTR(f.upper_buffer, ==, lower);
    DTUNITTEST_ASSERT_PTR(f.lower_buffer, ==, upper);

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
static dterr_t*
test_dtflipper_flip_fails_when_busy(void)
{
    dterr_t* dterr = NULL;

    int upper[1], lower[1];
    dtflipper_t f;
    void* p = NULL;

    dterr = dtflipper_init(&f, upper, lower);
    DTUNITTEST_ASSERT_PTR(dterr, ==, NULL);

    dterr = dtflipper_control_upper(&f, &p);
    DTUNITTEST_ASSERT_PTR(dterr, ==, NULL);
    dterr = dtflipper_flip(&f);
    DTUNITTEST_ASSERT_PTR(dterr, !=, NULL);
    dterr_dispose(dterr);
    dterr = NULL;

    dterr = dtflipper_release_upper(&f);
    DTUNITTEST_ASSERT_PTR(dterr, ==, NULL);

    dterr = dtflipper_control_lower(&f, &p);
    DTUNITTEST_ASSERT_PTR(dterr, ==, NULL);
    dterr = dtflipper_flip(&f);
    DTUNITTEST_ASSERT_PTR(dterr, !=, NULL);
    dterr_dispose(dterr);

    dterr = dtflipper_release_lower(&f);
    DTUNITTEST_ASSERT_PTR(dterr, ==, NULL);
    dterr = dtflipper_flip(&f);
    DTUNITTEST_ASSERT_PTR(dterr, ==, NULL);

    dterr = NULL;

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
static dterr_t*
test_dtflipper_control_busy_behavior(void)
{
    dterr_t* dterr = NULL;
    int upper[1], lower[1];
    dtflipper_t f;
    void* p = NULL;

    dterr = dtflipper_init(&f, upper, lower);
    DTUNITTEST_ASSERT_PTR(dterr, ==, NULL);

    // Mark upper as busy
    dterr = dtflipper_control_upper(&f, &p);
    DTUNITTEST_ASSERT_PTR(dterr, ==, NULL);
    dterr = dtflipper_control_upper(&f, &p);
    DTUNITTEST_ASSERT_PTR(dterr, !=, NULL);
    dterr_dispose(dterr);
    dterr = NULL;

    // Mark lower as busy
    dterr = dtflipper_control_lower(&f, &p);
    DTUNITTEST_ASSERT_PTR(dterr, ==, NULL);
    dterr = dtflipper_control_lower(&f, &p);
    DTUNITTEST_ASSERT_PTR(dterr, !=, NULL);
    dterr_dispose(dterr);
    dterr = NULL;

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
// suite runner
void
test_dtflipper(DTUNITTEST_SUITE_ARGS)
{
    DTUNITTEST_RUN_TEST(test_dtflipper_init_and_control_release);
    DTUNITTEST_RUN_TEST(test_dtflipper_flip_success);
    DTUNITTEST_RUN_TEST(test_dtflipper_flip_fails_when_busy);
    DTUNITTEST_RUN_TEST(test_dtflipper_control_busy_behavior);
}
