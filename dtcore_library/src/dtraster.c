#include "stdint.h"
#include <inttypes.h>

#include <dtcore/dterr.h>

#include <dtcore/dtvtable.h>

#include <dtcore/dtraster.h>

#define DTRASTER_MAX_VTABLES 8

static int32_t dtraster_model_numbers[DTRASTER_MAX_VTABLES] = { 0 };
static void* dtraster_vtables[DTRASTER_MAX_VTABLES] = { 0 };

static dtvtable_registry_t dtraster_registry = {
    .model_numbers = dtraster_model_numbers,
    .vtables = dtraster_vtables,
    .max_vtables = DTRASTER_MAX_VTABLES,
};

// --------------------------------------------------------------------------------------------
dterr_t*
dtraster_set_vtable(int32_t model_number, dtraster_vt_t* vtable)
{
    dterr_t* dterr = dtvtable_set(&dtraster_registry, model_number, (void*)vtable);
    if (dterr != NULL)
        dterr =
          dterr_new(DTERR_FAIL, DTERR_LOC, dterr, "setting dtraster vtable failed for model number %" PRId32, model_number);

    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtraster_get_vtable(int32_t model_number, dtraster_vt_t** vtable)
{
    dterr_t* dterr = dtvtable_get(&dtraster_registry, model_number, (void**)vtable);
    if (dterr != NULL)
        dterr =
          dterr_new(DTERR_FAIL, DTERR_LOC, dterr, "getting dtraster vtable failed for model number %" PRId32, model_number);

    return dterr;
}

DTVTABLE_DISPATCH(dtraster, blit, DTRASTER_BLIT_ARGS, DTRASTER_BLIT_PARAMS, dterr_t*)
DTVTABLE_DISPATCH(dtraster,
  create_compatible_raster,
  DTRASTER_CREATE_COMPATIBLE_RASTER_ARGS,
  DTRASTER_CREATE_COMPATIBLE_RASTER_PARAMS,
  dterr_t*)
DTVTABLE_DISPATCH(dtraster, use_buffer, DTRASTER_USE_BUFFER_ARGS, DTRASTER_USE_BUFFER_PARAMS, dterr_t*)
DTVTABLE_DISPATCH(dtraster, new_buffer, DTRASTER_NEW_BUFFER_ARGS, DTRASTER_NEW_BUFFER_PARAMS, dterr_t*)
DTVTABLE_DISPATCH(dtraster, get_shape, DTRASTER_GET_SHAPE_ARGS, DTRASTER_GET_SHAPE_PARAMS, dterr_t*)
DTVTABLE_DISPATCH(dtraster, fetch_pixel, DTRASTER_FETCH_PIXEL_ARGS, DTRASTER_FETCH_PIXEL_PARAMS, dterr_t*)
DTVTABLE_DISPATCH(dtraster, store_pixel, DTRASTER_STORE_PIXEL_ARGS, DTRASTER_STORE_PIXEL_PARAMS, dterr_t*)

// --------------------------------------------------------------------------------------------
void
dtraster_dispose(dtraster_handle handle)
{
    if (handle == NULL)
        return;
    int32_t model_number = *((int32_t*)handle);
    dtraster_vt_t* vtable;
    if (dtraster_get_vtable(model_number, &vtable) == NULL)
        vtable->dispose((void*)handle);
}
