#include "stdint.h"
#include <inttypes.h>

#include <dtcore/dterr.h>

#include <dtcore/dtvtable.h>

#include <dtcore/dtglyph.h>
#include <dtcore/dtraster.h>
#include <dtcore/dtrgba8888.h>

#define DTGLYPH_MAX_VTABLES 8

static int32_t dtglyph_model_numbers[DTGLYPH_MAX_VTABLES] = { 0 };
static void* dtglyph_vtables[DTGLYPH_MAX_VTABLES] = { 0 };

static dtvtable_registry_t dtglyph_registry = {
    .model_numbers = dtglyph_model_numbers,
    .vtables = dtglyph_vtables,
    .max_vtables = DTGLYPH_MAX_VTABLES,
};

// --------------------------------------------------------------------------------------------
dterr_t*
dtglyph_set_vtable(int32_t model_number, dtglyph_vt_t* vtable)
{
    dterr_t* dterr = dtvtable_set(&dtglyph_registry, model_number, (void*)vtable);
    if (dterr != NULL)
        dterr =
          dterr_new(DTERR_FAIL, DTERR_LOC, dterr, "setting dtglyph vtable failed for model number %" PRId32, model_number);

    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtglyph_get_vtable(int32_t model_number, dtglyph_vt_t** vtable)
{
    dterr_t* dterr = dtvtable_get(&dtglyph_registry, model_number, (void**)vtable);
    if (dterr != NULL)
        dterr =
          dterr_new(DTERR_FAIL, DTERR_LOC, dterr, "getting dtglyph vtable failed for model number %" PRId32, model_number);

    return dterr;
}

DTVTABLE_DISPATCH(dtglyph, blit, DTGLYPH_BLIT_ARGS, DTGLYPH_BLIT_PARAMS, dterr_t*)
DTVTABLE_DISPATCH(dtglyph, calculate_box, DTGLYPH_CALCULATE_BOX_ARGS, DTGLYPH_CALCULATE_BOX_PARAMS, dterr_t*)

// --------------------------------------------------------------------------------------------
void
dtglyph_dispose(dtglyph_handle handle)
{
    int32_t model_number = *((int32_t*)handle);
    dtglyph_vt_t* vtable;
    if (dtglyph_get_vtable(model_number, &vtable) == NULL)
        vtable->dispose((void*)handle);
}
