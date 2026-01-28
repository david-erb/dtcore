
#include <stdlib.h> // malloc, free
#include <string.h> // memset

#include <dtcore/dterr.h>
#include <dtcore/dtguid.h>
#include <dtcore/dtlog.h>
#include <dtcore/dtpackable.h>
#include <dtcore/dtpackx.h>

#include <dtcore/dtguidable.h>
#include <dtcore/dtguidable_pool.h>

#define TAG "dtguidable_pool"

// --------------------------------------------------------------------------------------------
dterr_t*
dtguidable_pool_init(dtguidable_pool_t* self, int max_items)
{
    dterr_t* dterr = NULL;

    if (self == NULL || max_items <= 0)
    {
        dterr = dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "invalid arguments");
        goto cleanup;
    }

    self->items = (dtguidable_handle*)calloc((size_t)max_items, sizeof(dtguidable_handle));
    if (self->items == NULL)
    {
        dterr = dterr_new(DTERR_NOMEM, DTERR_LOC, NULL, "calloc failed");
        goto cleanup;
    }

    self->max_items = max_items;

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
void
dtguidable_pool_dispose(dtguidable_pool_t* self)
{
    if (self == NULL)
        return;

    if (self->items != NULL)
    {
        free(self->items);
    }

    memset(self, 0, sizeof(dtguidable_pool_t)); // Clear the structure
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtguidable_pool_insert(dtguidable_pool_t* self, dtguidable_handle item)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(item);

    // First, check if the item is already inserted
    dtguid_t candidate_guid = { 0 };
    DTERR_C(dtguidable_get_guid(item, &candidate_guid));

    dtguidable_handle existing_item;
    DTERR_C(dtguidable_pool_search(self, &candidate_guid, &existing_item));
    if (existing_item != NULL)
    {
        dterr = dterr_new(DTERR_EXISTS, DTERR_LOC, NULL, "item already exists in the dtguidable pool");
        goto cleanup;
    }

    // Otherwise, find a free slot
    for (int i = 0; i < self->max_items; ++i)
    {
        if (self->items[i] == NULL)
        {
            self->items[i] = item;
            goto cleanup;
        }
    }

    // No free slot
    dterr = dterr_new(DTERR_NOMEM, DTERR_LOC, NULL, "dtguidable pool is full");

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtguidable_pool_search(dtguidable_pool_t* self, dtguid_t* guid, dtguidable_handle* item)
{
    dterr_t* dterr = NULL;
    DTERR_ASSERT_NOT_NULL(self);
    DTERR_ASSERT_NOT_NULL(guid);
    DTERR_ASSERT_NOT_NULL(item);

    for (int i = 0; i < self->max_items; ++i)
    {
        if (self->items[i] != NULL)
        {
            dtguid_t candidate_guid;
            DTERR_C(dtguidable_get_guid(self->items[i], &candidate_guid));

            if (dtguid_is_equal(&candidate_guid, guid))
            {
                *item = self->items[i];
                goto cleanup;
            }
        }
    }

    *item = NULL; // Not found, no error

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtguidable_pool_count(dtguidable_pool_t* self, int32_t* count)
{
    dterr_t* dterr = NULL;

    if (self == NULL)
    {
        dterr = dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "invalid arguments");
        goto cleanup;
    }

    *count = 0;

    for (int i = 0; i < self->max_items; ++i)
    {
        if (self->items[i] != NULL)
        {
            (*count)++;
        }
    }

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
dterr_t*
dtguidable_pool_remove(dtguidable_pool_t* self, dtguidable_handle item)
{
    dterr_t* dterr = NULL;

    if (self == NULL || item == NULL)
    {
        dterr = dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "invalid arguments");
        goto cleanup;
    }

    dtguid_t guid;
    DTERR_C(dtguidable_get_guid(item, &guid));

    for (int i = 0; i < self->max_items; ++i)
    {
        if (self->items[i] != NULL)
        {
            dtguid_t candidate_guid;
            DTERR_C(dtguidable_get_guid(self->items[i], &candidate_guid));

            if (dtguid_is_equal(&candidate_guid, &guid))
            {
                self->items[i] = NULL;
                goto cleanup; // Found and removed
            }
        }

        // If not found, silently succeed
    }

cleanup:
    return dterr;
}

// --------------------------------------------------------------------------------------------
// packs the items in the pool into the output buffer
// presumes each item implements dtpackable

dterr_t*
dtguidable_pool_packx(dtguidable_pool_t* pool, uint8_t* output, int32_t* offset, int32_t length)
{
    dterr_t* dterr = NULL;

    if (pool == NULL || output == NULL)
        goto cleanup;

    for (int i = 0; i < pool->max_items; i++)
    {
        dtpackable_handle handle = (dtpackable_handle)(pool->items[i]);
        if (handle != NULL)
        {
            DTERR_C(dtpackable_packx(handle, output, offset, length));
        }
    }

cleanup:
    if (dterr)
    {
        dterr = dterr_new(DTERR_FAIL, DTERR_LOC, dterr, "failed to pack dtguidable_pool items");
    }

    return dterr;
}
