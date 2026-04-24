#include "stdint.h"
#include <inttypes.h>

#include <dtcore/dterr.h>

#include <dtcore/dtvtable.h>

#include <dtcore/dtrpc.h>

#define DTRPC_MAX_VTABLES 16

static int32_t dtrpc_model_numbers[DTRPC_MAX_VTABLES] = { 0 };
static void* dtrpc_vtables[DTRPC_MAX_VTABLES] = { 0 };

static dtvtable_registry_t dtrpc_registry = {
    .model_numbers = dtrpc_model_numbers,
    .vtables = dtrpc_vtables,
    .max_vtables = DTRPC_MAX_VTABLES,
};

// --------------------------------------------------------------------------------------------
dterr_t*
dtrpc_set_vtable(int32_t model_number, dtrpc_vt_t* vtable)
{
    dterr_t* dterr = dtvtable_set(&dtrpc_registry, model_number, (void*)vtable);
    if (dterr != NULL)
        dterr = dterr_new(DTERR_FAIL, DTERR_LOC, dterr, "setting dtrpc vtable failed for model number %" PRId32, model_number);

    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtrpc_get_vtable(int32_t model_number, dtrpc_vt_t** vtable)
{
    dterr_t* dterr = dtvtable_get(&dtrpc_registry, model_number, (void**)vtable);
    if (dterr != NULL)
        dterr = dterr_new(DTERR_FAIL, DTERR_LOC, dterr, "getting dtrpc vtable failed for model number %" PRId32, model_number);

    return dterr;
}

DTVTABLE_DISPATCH(dtrpc, call, DTRPC_CALL_ARGS, DTRPC_CALL_PARAMS, dterr_t*)

// --------------------------------------------------------------------------------------------
void
dtrpc_dispose(dtrpc_handle handle)
{
    if (handle == NULL)
        return;
    int32_t model_number = *((int32_t*)handle);
    dtrpc_vt_t* vtable;
    if (dtrpc_get_vtable(model_number, &vtable) == NULL)
        vtable->dispose((void*)handle);
}
