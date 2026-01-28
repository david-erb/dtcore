#include "stdint.h"
#include <inttypes.h>

#include <dtcore/dterr.h>

#include <dtcore/dtvtable.h>

#include <dtcore/dtrandomizer_browngrav.h>
#include <dtcore/dtrandomizer_uniform.h>

#include <dtcore/dtrandomizer.h>

#define DT_RANDOMIZER_MAX_VTABLES 16

static int32_t dtrandomizer_model_numbers[DT_RANDOMIZER_MAX_VTABLES] = { 0 };
static void* dtrandomizer_vtables[DT_RANDOMIZER_MAX_VTABLES] = { 0 };

static dtvtable_registry_t dtrandomizer_registry = {
    .model_numbers = dtrandomizer_model_numbers,
    .vtables = dtrandomizer_vtables,
    .max_vtables = DT_RANDOMIZER_MAX_VTABLES,
};

// --------------------------------------------------------------------------------------------
dterr_t*
dtrandomizer_set_vtable(int32_t model_number, dtrandomizer_vt_t* vtable)
{
    dterr_t* dterr = dtvtable_set(&dtrandomizer_registry, model_number, (void*)vtable);
    if (dterr != NULL)
        dterr =
          dterr_new(DTERR_FAIL, DTERR_LOC, dterr, "setting dtrandomizer vtable failed for model number %" PRId32, model_number);

    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtrandomizer_get_vtable(int32_t model_number, dtrandomizer_vt_t** vtable)
{
    dterr_t* dterr = dtvtable_get(&dtrandomizer_registry, model_number, (void**)vtable);
    if (dterr != NULL)
        dterr =
          dterr_new(DTERR_FAIL, DTERR_LOC, dterr, "getting dtrandomizer vtable failed for model number %" PRId32, model_number);

    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtrandomizer_next(dtrandomizer_handle handle DTRANDOMIZER_NEXT_ARGS)
{
    int32_t model_number = *((int32_t*)handle);
    dtrandomizer_vt_t* vtable;
    dterr_t* dterr = dtrandomizer_get_vtable(model_number, &vtable);
    if (dterr != NULL)
        return dterr;
    return vtable->next((void*)handle DTRANDOMIZER_NEXT_PARAMS);
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtrandomizer_reset(dtrandomizer_handle handle DTRANDOMIZER_RESET_ARGS)
{
    int32_t model_number = *((int32_t*)handle);
    dtrandomizer_vt_t* vtable;
    dterr_t* dterr = dtrandomizer_get_vtable(model_number, &vtable);
    if (dterr != NULL)
        return dterr;
    return vtable->reset((void*)handle DTRANDOMIZER_RESET_PARAMS);
}

// --------------------------------------------------------------------------------------------
void
dtrandomizer_dispose(dtrandomizer_handle handle)
{
    int32_t model_number = *((int32_t*)handle);
    dtrandomizer_vt_t* vtable;
    if (dtrandomizer_get_vtable(model_number, &vtable) == NULL)
        vtable->dispose((void*)handle);
}
