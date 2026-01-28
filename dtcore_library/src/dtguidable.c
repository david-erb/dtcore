#include <inttypes.h>

#include <dtcore/dterr.h>
#include <dtcore/dtvtable.h>

#include <dtcore/dtguid.h>
#include <dtcore/dtguidable.h>

#define DTGUIDABLE_MAX_VTABLES 32

static int32_t dtguidable_model_numbers[DTGUIDABLE_MAX_VTABLES] = { 0 };
static void* dtguidable_vtables[DTGUIDABLE_MAX_VTABLES] = { 0 };

static dtvtable_registry_t dtguidable_registry = {
    .model_numbers = dtguidable_model_numbers,
    .vtables = dtguidable_vtables,
    .max_vtables = DTGUIDABLE_MAX_VTABLES,
};

// --------------------------------------------------------------------------------------------
dterr_t*
dtguidable_set_vtable(int32_t model_number, dtguidable_vt_t* vtable)
{
    dterr_t* dterr = dtvtable_set(&dtguidable_registry, model_number, (void*)vtable);
    if (dterr != NULL)
        dterr =
          dterr_new(DTERR_FAIL, DTERR_LOC, dterr, "setting dtguidable vtable failed for model number %" PRId32, model_number);

    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtguidable_get_vtable(int32_t model_number, dtguidable_vt_t** vtable)
{
    dterr_t* dterr = dtvtable_get(&dtguidable_registry, model_number, (void**)vtable);
    if (dterr != NULL)
        dterr =
          dterr_new(DTERR_FAIL, DTERR_LOC, dterr, "getting dtguidable vtable failed for model number %" PRId32, model_number);

    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtguidable_get_guid(const dtguidable_handle handle, dtguid_t* guid)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(handle);
    DTERR_ASSERT_NOT_NULL(guid);
    int32_t model_number = *((int32_t*)handle);
    dtguidable_vt_t* vtable;
    DTERR_C(dtguidable_get_vtable(model_number, &vtable));
    DTERR_C(vtable->get_guid((void*)handle, guid));
cleanup:
    return dterr;
}
