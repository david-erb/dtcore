#include <string.h>

#include <dtcore/dterr.h>
#include <dtcore/dtheaper.h>
#include <dtcore/dtrpc.h>

#include <dtcore/dtrpc_registry.h>

// --------------------------------------------------------------------------------------
static dterr_t*
dtrpc_registry__allocate_handles(int32_t capacity, dtrpc_handle** handles)
{
    dterr_t* dterr = NULL;
    int32_t bytes = 0;

    DTERR_ASSERT_NOT_NULL(handles);
    *handles = NULL;

    if (capacity <= 0)
    {
        return dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "capacity must be > 0");
    }

    bytes = (int32_t)(sizeof(dtrpc_handle) * (size_t)capacity);

    DTERR_C(dtheaper_alloc_and_zero(bytes, "dtrpc_handles", (void**)handles));

cleanup:

    return dterr;
}

// --------------------------------------------------------------------------------------
dterr_t*
dtrpc_registry_create(dtrpc_registry_t** self_ptr)
{
    dterr_t* dterr = NULL;
    dtrpc_registry_t* self = NULL;

    DTERR_ASSERT_NOT_NULL(self_ptr);
    *self_ptr = NULL;

    DTERR_C(dtheaper_alloc((int32_t)sizeof(dtrpc_registry_t), "dtrpc_registry_t", (void**)&self));
    memset(self, 0, sizeof(dtrpc_registry_t));

    DTERR_C(dtrpc_registry__allocate_handles(DTRPC_REGISTRY_INITIAL_CAPACITY, &self->rpc_handles));

    self->count = 0;
    self->capacity = DTRPC_REGISTRY_INITIAL_CAPACITY;

    *self_ptr = self;
    self = NULL;

cleanup:
    if (self != NULL)
    {
        dtrpc_registry_dispose(self);
    }

    return dterr;
}

// --------------------------------------------------------------------------------------
dterr_t*
dtrpc_registry_add(dtrpc_registry_t* self, dtrpc_handle handle)
{
    dterr_t* dterr = NULL;

    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(handle);

    if (self->count >= self->capacity)
    {
        dtrpc_handle* new_handles = NULL;
        int32_t new_capacity = 0;
        int32_t copy_byte_count = 0;

        new_capacity = self->capacity + DTRPC_REGISTRY_GROWTH;
        copy_byte_count = (int32_t)(sizeof(dtrpc_handle) * (size_t)self->count);

        DTERR_C(dtrpc_registry__allocate_handles(new_capacity, &new_handles));
        memcpy(new_handles, self->rpc_handles, (size_t)copy_byte_count);

        dtheaper_free(self->rpc_handles);
        self->rpc_handles = new_handles;
        self->capacity = new_capacity;
    }

    self->rpc_handles[self->count] = handle;
    self->count += 1;

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------
dterr_t*
dtrpc_registry_get(dtrpc_registry_t* self, int32_t index, dtrpc_handle* handle)
{
    dterr_t* dterr = NULL;

    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(handle);

    *handle = NULL;

    if (index < 0)
    {
        goto cleanup;
    }

    if (index >= self->count)
    {
        goto cleanup;
    }

    *handle = self->rpc_handles[index];

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------
void
dtrpc_registry_dispose(dtrpc_registry_t* self)
{
    if (self == NULL)
    {
        return;
    }

    dtheaper_free(self->rpc_handles);

    dtheaper_free(self);
}