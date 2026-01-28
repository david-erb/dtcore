#pragma once

#include <dtcore/dterr.h>

#include <dtcore/dtguid.h>
#include <dtcore/dtguidable.h>

typedef struct dtguidable_pool_t
{
    int max_items;
    dtguidable_handle* items;
} dtguidable_pool_t;

extern dterr_t*
dtguidable_pool_init(dtguidable_pool_t* pool, int max_items);

extern void
dtguidable_pool_dispose(dtguidable_pool_t* pool);

extern dterr_t*
dtguidable_pool_insert(dtguidable_pool_t* pool, dtguidable_handle item);

extern dterr_t*
dtguidable_pool_search(dtguidable_pool_t* pool, dtguid_t* guid, dtguidable_handle* item);

extern dterr_t*
dtguidable_pool_count(dtguidable_pool_t* pool, int32_t* count);

extern dterr_t*
dtguidable_pool_remove(dtguidable_pool_t* pool, dtguidable_handle item);

extern dterr_t*
dtguidable_pool_packx(dtguidable_pool_t* pool, uint8_t* output, int32_t* offset, int32_t length);
