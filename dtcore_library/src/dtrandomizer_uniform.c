
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <dtcore/dtcore_constants.h>

#include <dtcore/dterr.h>
#include <dtcore/dtlcg.h>
#include <dtcore/dtlog.h>
#include <dtcore/dtobject.h>
#include <dtcore/dtpackable.h>

#include <dtcore/dtrandomizer.h>
#include <dtcore/dtrandomizer_uniform.h>

DTRANDOMIZER_INIT_VTABLE(dtrandomizer_uniform);
DTOBJECT_INIT_VTABLE(dtrandomizer_uniform);
DTPACKABLE_INIT_VTABLE(dtrandomizer_uniform);

#define TAG "dtrandomizer_uniform"

// --------------------------------------------------------------------------------------------
dterr_t*
dtrandomizer_uniform_create(dtrandomizer_uniform_t** self)
{
  dterr_t* dterr = NULL;

  void* space = malloc(sizeof(dtrandomizer_uniform_t));
  if (space == NULL) {
    dterr = dterr_new(DTERR_NOMEM,
                      DTERR_LOC,
                      NULL,
                      "failed to allocate memory for dtrandomizer_uniform_t");
    goto cleanup;
  }

  *self = (dtrandomizer_uniform_t*)space;

  DTERR_C(dtrandomizer_uniform_init(*self));

  (*self)->is_malloced = true;

cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
static bool vtables_are_registered = false;

dterr_t*
dtrandomizer_uniform_register_vtables(void)
{
  dterr_t* dterr = NULL;

  if (!vtables_are_registered) {
    int32_t model_number = DTCORE_CONSTANTS_RANDOMIZER_MODEL_UNIFORM;

    DTERR_C(dtrandomizer_set_vtable(model_number,
                                    &dtrandomizer_uniform_randomizer_vt));
    DTERR_C(dtobject_set_vtable(model_number, &dtrandomizer_uniform_object_vt));
    DTERR_C(
      dtpackable_set_vtable(model_number, &dtrandomizer_uniform_packable_vt));

    vtables_are_registered = true;
  }

cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtrandomizer_uniform_init(dtrandomizer_uniform_t* self)
{
  dterr_t* dterr = NULL;

  memset(self, 0, sizeof(*self));
  self->model_number = DTCORE_CONSTANTS_RANDOMIZER_MODEL_UNIFORM;

  // ensure vtables are registered for this model number
  DTERR_C(dtrandomizer_uniform_register_vtables());

cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtrandomizer_uniform_config(dtrandomizer_uniform_t* self,
                            dtrandomizer_uniform_config_t* configuration)
{
  dterr_t* dterr = NULL;

  self->config = *configuration;

  dterr = dtrandomizer_uniform_reset(self);

  return dterr;
}

// --------------------------------------------------------------------------------------------
static dtrandomizer_uniform_metrics_t metrics = { 0 };
dtrandomizer_uniform_metrics_t*
dtrandomizer_uniform_metrics_fetch(void)
{
  return &metrics;
}

dterr_t*
dtrandomizer_uniform_next(dtrandomizer_uniform_t* self, int32_t* value)
{
  dterr_t* dterr = NULL;

  metrics.next_metric1++;

  if (self->config.edge == 0 || self->config.step == 0) {
    *value = self->config.home;
    return dterr;
  }

  metrics.next_metric2++;

  // Generate a random step in the range [-step, step]
  // int32_t step = (rand() % (2 * self->config.step + 1)) - self->config.step;
  int32_t step =
    dtlcg32_next_range(&self->lcg, -self->config.step, self->config.step);

  // Compute the next value
  int32_t here = self->_last + step;

  // Clamp the value within [Home - Edge, Home + Edge]
  if (here < (self->config.home - self->config.edge))
    here = self->config.home - self->config.edge;
  if (here > (self->config.home + self->config.edge))
    here = self->config.home + self->config.edge;

  self->_last = here;
  *value = here;

  return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtrandomizer_uniform_reset(dtrandomizer_uniform_t* self)
{
  dterr_t* dterr = NULL;

  // if (self->config.seed == 0)
  //     srand((unsigned int)time(NULL)); // Use time-based seed
  // else
  //     srand((unsigned int)self->config.seed); // Use configured seed

  if (self->config.seed == 0)
    dtlcg32_seed(&self->lcg, (unsigned int)time(NULL)); // Use time-based seed
  else
    dtlcg32_seed(&self->lcg,
                 (unsigned int)self->config.seed); // Use configured seed

  self->_last = self->config.home;

  return dterr;
}
// --------------------------------------------------------------------------------------------
// Returns packed length
dterr_t*
dtrandomizer_uniform_packx_length(
  dtrandomizer_uniform_t* self DTPACKABLE_PACKX_LENGTH_ARGS)
{
  (void)self;
  *length = 5 * dtpackx_pack_int32_length();
  return NULL;
}

// --------------------------------------------------------------------------------------------
// Packs the uniform randomizer values into the output buffer
dterr_t*
dtrandomizer_uniform_packx(dtrandomizer_uniform_t* self DTPACKABLE_PACKX_ARGS)
{
  dterr_t* dterr = NULL;
  int32_t p = offset ? *offset : 0;
  int32_t n = 0;

  n = dtpackx_pack_int32(self->model_number, output, p, length);
  if (n < 0)
    goto cleanup;
  p += n;

  n = dtpackx_pack_int32(self->config.home, output, p, length);
  if (n < 0)
    goto cleanup;
  p += n;

  n = dtpackx_pack_int32(self->config.step, output, p, length);
  if (n < 0)
    goto cleanup;
  p += n;

  n = dtpackx_pack_int32(self->config.edge, output, p, length);
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
// Unpacks the uniform randomizer values from the input buffer
dterr_t*
dtrandomizer_uniform_unpackx(
  dtrandomizer_uniform_t* self DTPACKABLE_UNPACKX_ARGS)
{
  dterr_t* dterr = NULL;
  int32_t p = offset ? *offset : 0;
  int32_t n = 0;

  n = dtpackx_unpack_int32(input, p, length, &self->model_number);
  if (n < 0)
    goto cleanup;
  p += n;

  n = dtpackx_unpack_int32(input, p, length, &self->config.home);
  if (n < 0)
    goto cleanup;
  p += n;

  n = dtpackx_unpack_int32(input, p, length, &self->config.step);
  if (n < 0)
    goto cleanup;
  p += n;

  n = dtpackx_unpack_int32(input, p, length, &self->config.edge);
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
  DTERR_C(dtrandomizer_uniform_reset(self));

cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
// Validates the device configuration
dterr_t*
dtrandomizer_uniform_validate_unpacked(dtrandomizer_uniform_t* self)
{
  dterr_t* dterr = NULL;

  if (self->model_number != DTCORE_CONSTANTS_RANDOMIZER_MODEL_UNIFORM) {
    dterr = dterr_new(DTERR_BADCONFIG,
                      DTERR_LOC,
                      NULL,
                      "model_number was %d but must be "
                      "DTCORE_CONSTANTS_RANDOMIZER_MODEL_UNIFORM (%d)",
                      self->model_number,
                      DTCORE_CONSTANTS_RANDOMIZER_MODEL_UNIFORM);
    goto cleanup;
  }

cleanup:
  return dterr;
}

// --------------------------------------------------------------------------------------------
// dtobject implementation
// --------------------------------------------------------------------------------------------
void
dtrandomizer_uniform_copy(dtrandomizer_uniform_t* self,
                          dtrandomizer_uniform_t* that)
{
  self->config = that->config;
  self->model_number = that->model_number;
  self->_last = that->_last;
}

// --------------------------------------------------------------------------------------------
void
dtrandomizer_uniform_dispose(dtrandomizer_uniform_t* self)
{
  if (self->is_malloced)
    free(self);
  else
    memset(self, 0, sizeof(*self));
}

// --------------------------------------------------------------------------------------------
// Equality check
bool
dtrandomizer_uniform_equals(dtrandomizer_uniform_t* a,
                            dtrandomizer_uniform_t* b)
{
  if (a == NULL || b == NULL) {
    return false;
  }
  return a->model_number == b->model_number && //
         a->config.home == b->config.home &&   //
         a->config.step == b->config.step &&   //
         a->config.edge == b->config.edge &&   //
         a->config.seed == b->config.seed;
}

// --------------------------------------------------------------------------------------------
const char*
dtrandomizer_uniform_get_class(dtrandomizer_uniform_t* self)
{
  return "dtrandomizer_uniform_t";
}

// --------------------------------------------------------------------------------------------

bool
dtrandomizer_uniform_is_iface(dtrandomizer_uniform_t* self,
                              const char* iface_name)
{
  return strcmp(iface_name, "dtrandomizer_iface") == 0 ||
         strcmp(iface_name, "dtpackable_iface") == 0 ||
         strcmp(iface_name, "dtobject_iface") == 0;
}

// --------------------------------------------------------------------------------------------
// Converts the lumen values to a string (for debugging)
void
dtrandomizer_uniform_to_string(dtrandomizer_uniform_t* self,
                               char* buffer,
                               size_t buffer_size)
{
  snprintf(buffer,
           buffer_size,
           "#%d, home=%d, step=%d, edge=%d, seed=%d",
           (int)self->model_number,
           (int)self->config.home,
           (int)self->config.step,
           (int)self->config.edge,
           (int)self->config.seed);
}
