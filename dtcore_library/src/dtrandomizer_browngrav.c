#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <dtcore/dtcore_constants.h>

#include <dtcore/dterr.h>
#include <dtcore/dtobject.h>
#include <dtcore/dtpackable.h>

#include <dtcore/dtrandomizer.h>
#include <dtcore/dtrandomizer_browngrav.h>

DTRANDOMIZER_INIT_VTABLE(dtrandomizer_browngrav);
DTPACKABLE_INIT_VTABLE(dtrandomizer_browngrav);

// --------------------------------------------------------------------------------------------
dterr_t*
dtrandomizer_browngrav_create(dtrandomizer_browngrav_t** self)
{
  dterr_t* dterr = NULL;

  void* space = malloc(sizeof(dtrandomizer_browngrav_t));
  if (space == NULL) {
    dterr = dterr_new(DTERR_NOMEM,
                      DTERR_LOC,
                      NULL,
                      "failed to allocate memory for dtrandomizer_browngrav_t");
    goto cleanup;
  }

  *self = (dtrandomizer_browngrav_t*)space;

  DTERR_C(dtrandomizer_browngrav_init(*self));

  (*self)->is_malloced = true;

cleanup:
  return dterr;
}
// --------------------------------------------------------------------------------------------
static bool vtables_are_registered = false;

dterr_t*
dtrandomizer_browngrav_register_vtables(void)
{
  dterr_t* dterr = NULL;

  if (!vtables_are_registered) {
    int32_t model_number = DTCORE_CONSTANTS_RANDOMIZER_MODEL_BROWNIAN_GRAV;

    DTERR_C(dtrandomizer_set_vtable(model_number,
                                    &dtrandomizer_browngrav_randomizer_vt));
    DTERR_C(
      dtpackable_set_vtable(model_number, &dtrandomizer_browngrav_packable_vt));

    vtables_are_registered = true;
  }

cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtrandomizer_browngrav_init(dtrandomizer_browngrav_t* self)
{
  dterr_t* dterr = NULL;

  memset(self, 0, sizeof(*self));
  self->model_number = DTCORE_CONSTANTS_RANDOMIZER_MODEL_BROWNIAN_GRAV;

  // ensure vtables are registered for this model number
  DTERR_C(dtrandomizer_browngrav_register_vtables());

cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtrandomizer_browngrav_config(dtrandomizer_browngrav_t* self,
                              dtrandomizer_browngrav_config_t* configuration)
{
  dterr_t* dterr = NULL;

  self->config = *configuration;

  dterr = dtrandomizer_browngrav_reset(self);

  return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtrandomizer_browngrav_next(dtrandomizer_browngrav_t* self, int32_t* value)
{
  dterr_t* dterr = NULL;

  if (self->config.noise_intensity == 0 ||
      self->config.attraction_strength == 0) {
    *value = self->config.attraction_point;
    return dterr;
  }

  // Compute attraction force with a gradual effect (like damping)
  double alpha =
    0.01 * self->config.attraction_strength; // Scale down attraction
  int32_t attraction_force =
    -alpha * (self->_last - self->config.attraction_point);

  // Add a strong random noise component for longer excursions
  int32_t noise = (rand() % (2 * self->config.noise_intensity + 1)) -
                  self->config.noise_intensity;

  // Compute the next position
  int32_t here = self->_last + attraction_force + noise;

  self->_last = here;
  *value = here;

  return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtrandomizer_browngrav_reset(dtrandomizer_browngrav_t* self)
{
  dterr_t* dterr = NULL;

  if (self->config.seed == 0)
    srand((unsigned int)time(NULL)); // Use time-based seed
  else
    srand((unsigned int)self->config.seed); // Use configured seed

  self->_last = self->config.attraction_point;

  return dterr;
}

// --------------------------------------------------------------------------------------------
// Returns packed length
dterr_t*
dtrandomizer_browngrav_packx_length(
  dtrandomizer_browngrav_t* self DTPACKABLE_PACKX_LENGTH_ARGS)
{
  (void)self;
  *length = 5 * dtpackx_pack_int32_length();
  return NULL;
}

// --------------------------------------------------------------------------------------------
// Packs the browngrav randomizer values into the output buffer
dterr_t*
dtrandomizer_browngrav_packx(
  dtrandomizer_browngrav_t* self DTPACKABLE_PACKX_ARGS)
{
  dterr_t* dterr = NULL;
  int32_t p = offset ? *offset : 0;
  int32_t n = 0;

  n = dtpackx_pack_int32(self->model_number, output, p, length);
  if (n < 0)
    goto cleanup;
  p += n;

  n = dtpackx_pack_int32(self->config.attraction_point, output, p, length);
  if (n < 0)
    goto cleanup;
  p += n;

  n = dtpackx_pack_int32(self->config.attraction_strength, output, p, length);
  if (n < 0)
    goto cleanup;
  p += n;

  n = dtpackx_pack_int32(self->config.noise_intensity, output, p, length);
  if (n < 0)
    goto cleanup;
  p += n;

  n = dtpackx_pack_int32(self->config.seed, output, p, length);
  if (n < 0)
    goto cleanup;
  p += n;

  if (offset)
    *offset = p;

cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
// Unpacks the browngrav randomizer values from the input buffer
dterr_t*
dtrandomizer_browngrav_unpackx(
  dtrandomizer_browngrav_t* self DTPACKABLE_UNPACKX_ARGS)
{
  dterr_t* dterr = NULL;
  int32_t p = offset ? *offset : 0;
  int32_t n = 0;

  n = dtpackx_unpack_int32(input, p, length, &self->model_number);
  if (n < 0)
    goto cleanup;
  p += n;

  n = dtpackx_unpack_int32(input, p, length, &self->config.attraction_point);
  if (n < 0)
    goto cleanup;
  p += n;

  n = dtpackx_unpack_int32(input, p, length, &self->config.attraction_strength);
  if (n < 0)
    goto cleanup;
  p += n;

  n = dtpackx_unpack_int32(input, p, length, &self->config.noise_intensity);
  if (n < 0)
    goto cleanup;
  p += n;

  n = dtpackx_unpack_int32(input, p, length, &self->config.seed);
  if (n < 0)
    goto cleanup;
  p += n;

  if (offset)
    *offset = p;

  // unpack implies configure
  DTERR_C(dtrandomizer_browngrav_reset(self));

cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
// Validates the device configuration
dterr_t*
dtrandomizer_browngrav_validate_unpacked(dtrandomizer_browngrav_t* self)
{
  dterr_t* dterr = NULL;

  if (self->model_number != DTCORE_CONSTANTS_RANDOMIZER_MODEL_BROWNIAN_GRAV) {
    dterr = dterr_new(DTERR_BADCONFIG,
                      DTERR_LOC,
                      NULL,
                      "model_number was %d but must be "
                      "DTCORE_CONSTANTS_RANDOMIZER_MODEL_BROWNIAN_GRAV (%d)",
                      self->model_number,
                      DTCORE_CONSTANTS_RANDOMIZER_MODEL_BROWNIAN_GRAV);
    goto cleanup;
  }

cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
// Copy constructor
void
dtrandomizer_browngrav_copy(dtrandomizer_browngrav_t* self,
                            dtrandomizer_browngrav_t* that)
{
  self->config = that->config;
  self->model_number = that->model_number;
  self->_last = that->_last;
}

// --------------------------------------------------------------------------------------------
void
dtrandomizer_browngrav_dispose(dtrandomizer_browngrav_t* self)
{
  if (self->is_malloced)
    free(self);
  else
    memset(self, 0, sizeof(*self));
}

// --------------------------------------------------------------------------------------------
// Equality check
bool
dtrandomizer_browngrav_equals(dtrandomizer_browngrav_t* a,
                              dtrandomizer_browngrav_t* b)
{
  if (a == NULL || b == NULL) {
    return false;
  }
  return a->model_number == b->model_number &&                             //
         a->config.attraction_point == b->config.attraction_point &&       //
         a->config.attraction_strength == b->config.attraction_strength && //
         a->config.noise_intensity == b->config.noise_intensity &&         //
         a->config.seed == b->config.seed;
}

// --------------------------------------------------------------------------------------------
const char*
dtrandomizer_browngrav_get_class(dtrandomizer_browngrav_t* self)
{
  return "dtrandomizer_browngrav_t";
}

// --------------------------------------------------------------------------------------------

bool
dtrandomizer_browngrav_is_iface(dtrandomizer_browngrav_t* self,
                                const char* iface_name)
{
  return strcmp(iface_name, "dtrandomizer_iface") == 0 ||
         strcmp(iface_name, "dtpackable_iface") == 0 ||
         strcmp(iface_name, "dtobject_iface") == 0;
}

// --------------------------------------------------------------------------------------------
// Convert to string (for debugging)
void
dtrandomizer_browngrav_to_string(dtrandomizer_browngrav_t* self,
                                 char* buffer,
                                 size_t buffer_size)
{
  snprintf(buffer,
           buffer_size,
           "model_number=%d, attraction_point=%d, attraction_strength=%d, "
           "noise_intensity=%d, seed=%d",
           (int)self->model_number,
           (int)self->config.attraction_point,
           (int)self->config.attraction_strength,
           (int)self->config.noise_intensity,
           (int)self->config.seed);
}
