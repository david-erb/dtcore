#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <dtcore/dtcore_constants.h>

#include <dtcore/dterr.h>
#include <dtcore/dtpackable.h>

#include <dtcore/dtrandomizer_browngrav.h>
#include <dtcore/dtunittest.h>

// --------------------------------------------------------------------------------------------
// Unit test for packing and unpacking
dterr_t*
test_dtrandomizer_browngrav_pack_unpack(void)
{
  dterr_t* dterr = NULL;
  uint8_t* buffer = NULL;

  dtrandomizer_browngrav_t randomizer;
  dtrandomizer_browngrav_init(&randomizer);

  randomizer.model_number = DTCORE_CONSTANTS_RANDOMIZER_MODEL_BROWNIAN_GRAV;
  randomizer.config.attraction_point = 10;
  randomizer.config.attraction_strength = 5;
  randomizer.config.noise_intensity = 3;
  randomizer.config.seed = 12345;

  dtpackable_handle handle = (dtpackable_handle)&randomizer;
  int32_t pack_len;
  DTERR_C(dtpackable_packx_length(handle, &pack_len));
  buffer = malloc(pack_len);
  assert(buffer != NULL);
  memset(buffer, 0, pack_len);

  int32_t offset = 0;

  DTERR_C(dtpackable_packx(handle, buffer, &offset, pack_len));
  DTUNITTEST_ASSERT_INT(offset, ==, pack_len);

  dtrandomizer_browngrav_t unpacked_randomizer;
  dtrandomizer_browngrav_init(&unpacked_randomizer);
  dtpackable_handle unpacked_handle = (dtpackable_handle)&unpacked_randomizer;
  int32_t bytes_consumed = 0;
  DTERR_C(
    dtpackable_unpackx(unpacked_handle, buffer, &bytes_consumed, pack_len));
  DTUNITTEST_ASSERT_INT(bytes_consumed, ==, pack_len);

  // Verify values
  DTUNITTEST_ASSERT_INT(
    unpacked_randomizer.model_number, ==, randomizer.model_number);
  DTUNITTEST_ASSERT_INT(unpacked_randomizer.config.attraction_point,
                        ==,
                        randomizer.config.attraction_point);
  DTUNITTEST_ASSERT_INT(unpacked_randomizer.config.attraction_strength,
                        ==,
                        randomizer.config.attraction_strength);
  DTUNITTEST_ASSERT_INT(unpacked_randomizer.config.noise_intensity,
                        ==,
                        randomizer.config.noise_intensity);
  DTUNITTEST_ASSERT_INT(
    unpacked_randomizer.config.seed, ==, randomizer.config.seed);

cleanup:
  if (buffer != NULL)
    free(buffer);

  return dterr;
}

// --------------------------------------------------------------------------------------------
// Unit test for validating unpacked configuration
dterr_t*
test_dtrandomizer_browngrav_validate_unpacked(void)
{
  dterr_t* dterr = NULL;

  dtrandomizer_browngrav_t valid_randomizer;
  dtrandomizer_browngrav_init(&valid_randomizer);

  dterr = dtrandomizer_browngrav_validate_unpacked(&valid_randomizer);
  DTUNITTEST_ASSERT_PTR(dterr, ==, NULL);

  dtrandomizer_browngrav_t invalid_randomizer;
  dtrandomizer_browngrav_init(&invalid_randomizer);
  invalid_randomizer.model_number = 999; // Invalid model number

  dterr = dtrandomizer_browngrav_validate_unpacked(&invalid_randomizer);
  DTUNITTEST_ASSERT_PTR(dterr, !=, NULL);

  if (dterr != NULL) {
    dterr_dispose(dterr);
    dterr = NULL;
  }

cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
// Unit test for reset function
dterr_t*
test_dtrandomizer_browngrav_reset(void)
{
  dterr_t* dterr = NULL;
  dtrandomizer_browngrav_t randomizer;

  dtrandomizer_browngrav_init(&randomizer);
  randomizer.config.attraction_point = 50;
  randomizer.config.seed = 42;

  dtrandomizer_browngrav_reset(&randomizer);

  DTUNITTEST_ASSERT_INT(
    randomizer._last, ==, randomizer.config.attraction_point);

cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
// Unit test for next function with zero attraction strength or noise intensity
dterr_t*
test_dtrandomizer_browngrav_next_with_zero_params(void)
{
  dterr_t* dterr = NULL;
  dtrandomizer_browngrav_t randomizer;
  int32_t value;

  dtrandomizer_browngrav_init(&randomizer);
  randomizer.config.attraction_point = 100;
  randomizer.config.attraction_strength = 0;
  randomizer.config.noise_intensity = 0;
  dtrandomizer_browngrav_reset(&randomizer);

  dtrandomizer_browngrav_next(&randomizer, &value);
  DTUNITTEST_ASSERT_INT(value, ==, 100);

cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
// Unit test to ensure next values exhibit long excursions before returning to
// attraction_point
dterr_t*
test_dtrandomizer_browngrav_next_behavior(void)
{
  dterr_t* dterr = NULL;
  dtrandomizer_browngrav_t randomizer;
  int32_t value;
  int32_t above_count = 0, below_count = 0, within_range_count = 0;
  int32_t excursion_threshold =
    20; // Define what is considered a long excursion

  // Initialize with moderate attraction and noise
  dtrandomizer_browngrav_init(&randomizer);
  randomizer.config.attraction_point = 50;
  randomizer.config.attraction_strength = 5; // Smooth attraction
  randomizer.config.noise_intensity = 10;    // Allow large excursions
  randomizer.config.seed = 123;
  dtrandomizer_browngrav_reset(&randomizer);

  for (int i = 0; i < 1000; i++) // Increased iterations to capture excursions
  {
    dtrandomizer_browngrav_next(&randomizer, &value);

    if (value > randomizer.config.attraction_point + excursion_threshold)
      above_count++;
    else if (value < randomizer.config.attraction_point - excursion_threshold)
      below_count++;
    else
      within_range_count++;
  }

  // Verify that excursions are happening
  DTUNITTEST_ASSERT_TRUE(
    above_count > 10); // Ensure some long excursions above attraction point
  DTUNITTEST_ASSERT_TRUE(
    below_count > 10); // Ensure some long excursions below attraction point
  DTUNITTEST_ASSERT_TRUE(within_range_count >
                         50); // Ensure values do not diverge too much

cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
// Unit test to ensure next function generates varying results over time
dterr_t*
test_dtrandomizer_browngrav_next_variability(void)
{
  dterr_t* dterr = NULL;
  dtrandomizer_browngrav_t randomizer;
  int32_t prev_value, current_value;
  int32_t change_count = 0;
  int32_t no_change_threshold = 3; // Maximum consecutive same values allowed
  int32_t variability_threshold =
    5; // Minimum number of significant changes expected

  dtrandomizer_browngrav_init(&randomizer);
  randomizer.config.attraction_point = 50;
  randomizer.config.attraction_strength = 5; // Smooth attraction
  randomizer.config.noise_intensity = 10;    // Allow more variability
  randomizer.config.seed = 1;
  dtrandomizer_browngrav_reset(&randomizer);

  prev_value = randomizer._last; // Initialize with reset value

  for (int i = 0; i < 100; i++) // Test over multiple iterations
  {
    dtrandomizer_browngrav_next(&randomizer, &current_value);

    if (current_value != prev_value)
      change_count++;

    prev_value = current_value;
  }

  // Ensure enough variation happens
  DTUNITTEST_ASSERT_INT(change_count, >, variability_threshold);
  DTUNITTEST_ASSERT_INT(
    change_count,
    <,
    (100 - no_change_threshold)); // Ensure it's not totally random noise

cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
static dterr_t*
test_dtrandomizer_browngrav_dtobject_iface(void)
{
  dterr_t* dterr = NULL;

  //     dtrandomizer_browngrav_t object1, object2, object1_copy;
  //     dtrandomizer_browngrav_init(&object1);
  //     dtrandomizer_browngrav_init(&object2);
  //     dtrandomizer_browngrav_init(&object1_copy);

  //     object1.config.attraction_point = 10;
  //     object1.config.attraction_strength = 1;
  //     object1.config.noise_intensity = 2;
  //     object1.config.seed = 123;
  //     dtrandomizer_browngrav_reset(&object1);

  //     object2.config.attraction_point = 20;
  //     object2.config.attraction_strength = 1;
  //     object2.config.noise_intensity = 2;
  //     object2.config.seed = 123;
  //     dtrandomizer_browngrav_reset(&object2);

  //     const dtobject_iface* iface =
  //     dtrandomizer_browngrav_get_object_iface();
  //     DTERR_C(dtunittest_test_dtobject_iface(iface, &object1, &object2,
  //     &object1_copy));

  // cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
// Runs all tests
void
test_dtrandomizer_browngrav(DTUNITTEST_SUITE_ARGS)
{
  DTUNITTEST_RUN_TEST(test_dtrandomizer_browngrav_pack_unpack);
  DTUNITTEST_RUN_TEST(test_dtrandomizer_browngrav_validate_unpacked);
  DTUNITTEST_RUN_TEST(test_dtrandomizer_browngrav_reset);
  DTUNITTEST_RUN_TEST(test_dtrandomizer_browngrav_next_with_zero_params);
  DTUNITTEST_RUN_TEST(test_dtrandomizer_browngrav_next_behavior);
  DTUNITTEST_RUN_TEST(test_dtrandomizer_browngrav_next_variability);
  DTUNITTEST_RUN_TEST(test_dtrandomizer_browngrav_dtobject_iface);
}
