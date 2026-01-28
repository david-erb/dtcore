#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include <dtcore/dterr.h>
#include <dtcore/dtvtable.h>

#include <dtcore/dtobject.h>

#define DT_OBJECT_MAX_VTABLES 16

static int32_t dtobject_model_numbers[DT_OBJECT_MAX_VTABLES] = { 0 };
static void* dtobject_vtables[DT_OBJECT_MAX_VTABLES] = { 0 };

static dtvtable_registry_t dtobject_registry = {
    .model_numbers = dtobject_model_numbers,
    .vtables = dtobject_vtables,
    .max_vtables = DT_OBJECT_MAX_VTABLES,
};

// --------------------------------------------------------------------------------------------
dterr_t*
dtobject_set_vtable(int32_t model_number, dtobject_vt_t* vtable)
{
    dterr_t* dterr = dtvtable_set(&dtobject_registry, model_number, (void*)vtable);
    if (dterr != NULL)
        dterr =
          dterr_new(DTERR_FAIL, DTERR_LOC, dterr, "setting dtobject vtable failed for model number %" PRId32, model_number);

    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtobject_get_vtable(int32_t model_number, dtobject_vt_t** vtable)
{
    dterr_t* dterr = dtvtable_get(&dtobject_registry, model_number, (void**)vtable);
    if (dterr != NULL)
        dterr =
          dterr_new(DTERR_FAIL, DTERR_LOC, dterr, "getting dtobject vtable failed for model number %" PRId32, model_number);

    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtobject_create(int32_t model_number, dtobject_handle* handle)
{
    dtobject_vt_t* vtable;
    dterr_t* dterr;
    dterr = dtobject_get_vtable(model_number, &vtable);
    if (dterr != NULL)
        return dterr_new(DTERR_FAIL, DTERR_LOC, dterr, "dtobject_create failed for model number %" PRId32, model_number);

    return vtable->create((void*)handle DTOBJECT_CREATE_PARAMS);
}

// --------------------------------------------------------------------------------------------
void
dtobject_copy(dtobject_handle handle, dtobject_handle that)
{
    int32_t model_number = *((int32_t*)handle);
    if (model_number != *((int32_t*)that))
        return;
    dtobject_vt_t* vtable;
    if (dtobject_get_vtable(model_number, &vtable) == NULL)
        vtable->copy((void*)handle, (void*)that);
}

// --------------------------------------------------------------------------------------------
void
dtobject_dispose(dtobject_handle handle DTOBJECT_DISPOSE_ARGS)
{
    if (handle == NULL)
        return;
    int32_t model_number = *((int32_t*)handle);
    dtobject_vt_t* vtable;
    if (dtobject_get_vtable(model_number, &vtable) == NULL)
        vtable->dispose((void*)handle DTOBJECT_DISPOSE_PARAMS);
}

// --------------------------------------------------------------------------------------------
bool
dtobject_equals(dtobject_handle handle, dtobject_handle that)
{
    int32_t model_number = *((int32_t*)handle);
    if (model_number != *((int32_t*)that))
        return false;
    dtobject_vt_t* vtable;
    if (dtobject_get_vtable(model_number, &vtable) != NULL)
        return false;
    return vtable->equals((void*)handle, (void*)that);
}

// --------------------------------------------------------------------------------------------
const char*
dtobject_get_class(dtobject_handle handle DTOBJECT_GET_CLASS_ARGS)
{
    int32_t model_number = *((int32_t*)handle);
    dtobject_vt_t* vtable;
    if (dtobject_get_vtable(model_number, &vtable) != NULL)
        return NULL;
    return vtable->get_class((void*)handle DTOBJECT_GET_CLASS_PARAMS);
}

// --------------------------------------------------------------------------------------------
bool
dtobject_is_iface(dtobject_handle handle DTOBJECT_IS_IFACE_ARGS)
{
    int32_t model_number = *((int32_t*)handle);
    dtobject_vt_t* vtable;
    if (dtobject_get_vtable(model_number, &vtable) != NULL)
        return false;
    return vtable->is_iface((void*)handle DTOBJECT_IS_IFACE_PARAMS);
}

// --------------------------------------------------------------------------------------------
void
dtobject_to_string(dtobject_handle handle DTOBJECT_TO_STRING_ARGS)
{
    int32_t model_number = *((int32_t*)handle);
    dtobject_vt_t* vtable;
    if (dtobject_get_vtable(model_number, &vtable) == NULL)
        vtable->to_string((void*)handle DTOBJECT_TO_STRING_PARAMS);
    else
        snprintf(buffer, buffer_size, "dtobject: model number %" PRId32 " not found", model_number);
}
