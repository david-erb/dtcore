#include <inttypes.h>

#include <dtcore/dterr.h>
#include <dtcore/dtlog.h>

#include <dtcore/dtvtable.h>

#include <dtcore/dtpackable.h>

#define DT_PACKABLE_MAX_VTABLES 16

static int32_t dtpackable_model_numbers[DT_PACKABLE_MAX_VTABLES] = { 0 };
static void* dtpackable_vtables[DT_PACKABLE_MAX_VTABLES] = { 0 };

static dtvtable_registry_t dtpackable_registry = {
    .model_numbers = dtpackable_model_numbers,
    .vtables = dtpackable_vtables,
    .max_vtables = DT_PACKABLE_MAX_VTABLES,
};

#define TAG "dtpackable"

// --------------------------------------------------------------------------------------------
dterr_t*
dtpackable_set_vtable(int32_t model_number, dtpackable_vt_t* vtable)
{
    dterr_t* dterr = dtvtable_set(&dtpackable_registry, model_number, (void*)vtable);
    if (dterr != NULL)
        dterr =
          dterr_new(DTERR_FAIL, DTERR_LOC, dterr, "setting dtpackable vtable failed for model number %" PRId32, model_number);

    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtpackable_get_vtable(int32_t model_number, dtpackable_vt_t** vtable)
{
    dterr_t* dterr = dtvtable_get(&dtpackable_registry, model_number, (void**)vtable);
    if (dterr != NULL)
        dterr =
          dterr_new(DTERR_FAIL, DTERR_LOC, dterr, "getting dtpackable vtable failed for model number %" PRId32, model_number);

    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtpackable_packx_length(dtpackable_handle handle DTPACKABLE_PACKX_LENGTH_ARGS)
{
    int32_t model_number = *((int32_t*)handle);
    dtpackable_vt_t* vtable;
    dterr_t* dterr = dtpackable_get_vtable(model_number, &vtable);
    if (dterr != NULL)
        return dterr;
    return vtable->packx_length((void*)handle DTPACKABLE_PACKX_LENGTH_PARAMS);
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtpackable_packx(dtpackable_handle handle DTPACKABLE_PACKX_ARGS)
{
    int32_t model_number = *((int32_t*)handle);
    dtpackable_vt_t* vtable;
    dterr_t* dterr = dtpackable_get_vtable(model_number, &vtable);
    if (dterr != NULL)
        return dterr;
    return vtable->packx((void*)handle DTPACKABLE_PACKX_PARAMS);
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtpackable_unpackx(dtpackable_handle handle DTPACKABLE_UNPACKX_ARGS)
{
    int32_t model_number = *((int32_t*)handle);
    dtpackable_vt_t* vtable;
    dterr_t* dterr = dtpackable_get_vtable(model_number, &vtable);
    if (dterr != NULL)
        return dterr;
    return vtable->unpackx((void*)handle DTPACKABLE_UNPACKX_PARAMS);
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtpackable_validate_unpacked(dtpackable_handle handle)
{
    int32_t model_number = *((int32_t*)handle);
    dtpackable_vt_t* vtable;
    dterr_t* dterr = dtpackable_get_vtable(model_number, &vtable);
    if (dterr != NULL)
        return dterr;
    return vtable->validate_unpacked((void*)handle);
}
