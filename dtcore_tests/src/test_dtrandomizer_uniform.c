#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <dtcore/dtcore_constants.h>

#include <dtcore/dterr.h>
#include <dtcore/dtlog.h>
#include <dtcore/dtpackable.h>

#include <dtcore/dtrandomizer_uniform.h>
#include <dtcore/dtunittest.h>

// --------------------------------------------------------------------------------------------
// Unit test for dtrandomizer_uniform_t
dterr_t*
test_dtrandomizer_uniform_as_packable(void)
{
  dterr_t* dterr = NULL;
  uint8_t* buffer = NULL;

  // Initialize with known values
  dtrandomizer_uniform_t randomizer;
  dtrandomizer_uniform_init(&randomizer);

  randomizer.model_number = DTCORE_CONSTANTS_RANDOMIZER_MODEL_UNIFORM;
  randomizer.config.home = 10;
  randomizer.config.step = 5;
  randomizer.config.edge = 20;
  randomizer.config.seed = 12345;

  dtpackable_handle packable_handle = (dtpackable_handle)&randomizer;
  // Allocate buffer for packing
  int32_t pack_len;
  DTERR_C(dtpackable_packx_length(packable_handle, &pack_len));
  buffer = malloc(pack_len);
  assert(buffer != NULL);
  memset(buffer, 0, pack_len);

  // Pack the randomizer values
  int32_t offset = 0;
  DTERR_C(dtpackable_packx(packable_handle, buffer, &offset, pack_len));
  DTUNITTEST_ASSERT_INT(offset, ==, pack_len);

  // Unpack into a new structure
  dtrandomizer_uniform_t unpacked_randomizer;
  dtrandomizer_uniform_init(&unpacked_randomizer);
  dtpackable_handle unpacked_handle = (dtpackable_handle)&unpacked_randomizer;
  int32_t bytes_consumed = 0;
  DTERR_C(
    dtpackable_unpackx(unpacked_handle, buffer, &bytes_consumed, pack_len));
  DTUNITTEST_ASSERT_INT(bytes_consumed, ==, pack_len);

  // Verify that all values match
  DTUNITTEST_ASSERT_INT(
    unpacked_randomizer.model_number, ==, randomizer.model_number);
  DTUNITTEST_ASSERT_INT(
    unpacked_randomizer.config.home, ==, randomizer.config.home);
  DTUNITTEST_ASSERT_INT(
    unpacked_randomizer.config.step, ==, randomizer.config.step);
  DTUNITTEST_ASSERT_INT(
    unpacked_randomizer.config.edge, ==, randomizer.config.edge);
  DTUNITTEST_ASSERT_INT(
    unpacked_randomizer.config.seed, ==, randomizer.config.seed);

cleanup:
  if (buffer != NULL) {
    free(buffer);
  }

  return dterr;
}

// --------------------------------------------------------------------------------------------

// Unit test for dtrandomizer_uniform_validate_unpacked
dterr_t*
test_dtrandomizer_uniform_validate_unpacked(void)
{
  dterr_t* dterr = NULL;

  // Test case: Valid configuration
  dtrandomizer_uniform_t valid_randomizer;
  dtrandomizer_uniform_init(&valid_randomizer);

  dterr = dtrandomizer_uniform_validate_unpacked(&valid_randomizer);
  DTUNITTEST_ASSERT_PTR(dterr, ==, NULL); // Expect no error

  // Test case: Invalid configuration (wrong model number)
  dtrandomizer_uniform_t invalid_randomizer;
  dtrandomizer_uniform_init(&invalid_randomizer);
  invalid_randomizer.model_number = 999; // Invalid model number

  dterr = dtrandomizer_uniform_validate_unpacked(&invalid_randomizer);
  DTUNITTEST_ASSERT_PTR(dterr, !=, NULL); // Expect an error

  // Free the error structure if allocated
  if (dterr != NULL) {
    dterr_dispose(dterr);
    dterr = NULL;
  }

cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
// Test Reset: Ensures it initializes correctly
dterr_t*
test_dtrandomizer_uniform_reset(void)
{
  dterr_t* dterr = NULL;
  dtrandomizer_uniform_t randomizer;

  // Set config values
  dtrandomizer_uniform_init(&randomizer);
  randomizer.config.home = 50;
  randomizer.config.seed = 42;

  // Call Reset
  dtrandomizer_uniform_reset(&randomizer);

  // Ensure last is set to home
  DTUNITTEST_ASSERT_INT(randomizer._last, ==, randomizer.config.home);

cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
// Test Next: Ensures Next behaves correctly when Edge or Step is zero
dterr_t*
test_dtrandomizer_uniform_next_with_zero_edge_or_step(void)
{
  dterr_t* dterr = NULL;
  dtrandomizer_uniform_t randomizer;
  int32_t value;

  // Case 1: Edge = 0, Step = 0 → Should always return Home
  dtrandomizer_uniform_init(&randomizer);
  randomizer.config.home = 100;
  randomizer.config.edge = 0;
  randomizer.config.step = 0;
  dtrandomizer_uniform_reset(&randomizer);

  dtrandomizer_uniform_next(&randomizer, &value);
  DTUNITTEST_ASSERT_INT(value, ==, 100);

cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
// Test Next: Ensures it remains within [Home - Edge, Home + Edge]
dterr_t*
test_dtrandomizer_uniform_next_bounds(void)
{
  dterr_t* dterr = NULL;
  dtrandomizer_uniform_t randomizer;
  int32_t value;

  // Case 2: Edge and Step set → Next should always be within bounds
  dtrandomizer_uniform_init(&randomizer);
  randomizer.config.home = 50;
  randomizer.config.edge = 10;
  randomizer.config.step = 5;
  randomizer.config.seed = 123;
  dtrandomizer_uniform_reset(&randomizer);

  for (int i = 0; i < 100; i++) // Run multiple iterations to catch errors
  {
    dtrandomizer_uniform_next(&randomizer, &value);
    DTUNITTEST_ASSERT_INT(
      value >= 40 && value <= 60, ==, 1); // 40 to 60 (Home ± Edge)
  }

cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
// Test Next: Ensures multiple calls to Next produce varying results
dterr_t*
test_dtrandomizer_uniform_next_variability(void)
{
  dterr_t* dterr = NULL;
  dtrandomizer_uniform_t randomizer;
  int32_t value1, value2;

  dtrandomizer_uniform_init(&randomizer);
  randomizer.config.home = 50;
  randomizer.config.edge = 10;
  randomizer.config.step = 5;
  randomizer.config.seed = 1;
  dtrandomizer_uniform_reset(&randomizer);

  dtrandomizer_uniform_next(&randomizer, &value1);
  dtrandomizer_uniform_next(&randomizer, &value2);

  // Ensure that the two calls give different results in most cases
  DTUNITTEST_ASSERT_INT(value1, !=, value2);

cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
// Test the dtobject_iface for dtrandomizer_uniform_t
static dterr_t*
test_dtrandomizer_uniform_dtobject_iface(void)
{
  dterr_t* dterr = NULL;

  //     dtrandomizer_uniform_t object1, object2, object1_copy;
  //     dtrandomizer_uniform_init(&object1);
  //     dtrandomizer_uniform_init(&object2);
  //     dtrandomizer_uniform_init(&object1_copy);

  //     object1.config.home = 100;
  //     object1.config.step = 10;
  //     object1.config.edge = 500;
  //     object1.config.seed = 1;
  //     dtrandomizer_uniform_reset(&object1);

  //     object2.config.home = 200; // different from object1
  //     object2.config.step = 10;
  //     object2.config.edge = 500;
  //     object2.config.seed = 1;
  //     dtrandomizer_uniform_reset(&object2);

  //     const dtobject_iface* iface = dtrandomizer_uniform_get_object_iface();
  //     DTERR_C(dtunittest_test_dtobject_iface(iface, &object1, &object2,
  //     &object1_copy));

  // cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
// suite runner
void
test_dtrandomizer_uniform(DTUNITTEST_SUITE_ARGS)
{
  DTUNITTEST_RUN_TEST(test_dtrandomizer_uniform_as_packable);
  DTUNITTEST_RUN_TEST(test_dtrandomizer_uniform_validate_unpacked);
  DTUNITTEST_RUN_TEST(test_dtrandomizer_uniform_reset);
  DTUNITTEST_RUN_TEST(test_dtrandomizer_uniform_next_with_zero_edge_or_step);
  DTUNITTEST_RUN_TEST(test_dtrandomizer_uniform_next_bounds);
  DTUNITTEST_RUN_TEST(test_dtrandomizer_uniform_next_variability);
  DTUNITTEST_RUN_TEST(test_dtrandomizer_uniform_dtobject_iface);
}
