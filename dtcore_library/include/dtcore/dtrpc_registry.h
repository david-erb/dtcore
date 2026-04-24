/*
 * dtrpc_registry -- Growable list of RPC handles.
 *
 * Maintains a heap-allocated array of dtrpc_handle values that grows in
 * fixed increments as handles are added.  Provides indexed retrieval and
 * bulk disposal of all registered handles.
 *
 * cdox v1.0.2
 */
#pragma once

#include <stdint.h>

#include <dtcore/dterr.h>
#include <dtcore/dtrpc.h>

#define DTRPC_REGISTRY_INITIAL_CAPACITY 10
#define DTRPC_REGISTRY_GROWTH 10

typedef struct dtrpc_registry_t
{
    dtrpc_handle* rpc_handles;
    int32_t count;
    int32_t capacity;
} dtrpc_registry_t;

extern dterr_t*
dtrpc_registry_create(dtrpc_registry_t** self_ptr);

extern dterr_t*
dtrpc_registry_add(dtrpc_registry_t* self, dtrpc_handle handle);

extern dterr_t*
dtrpc_registry_get(dtrpc_registry_t* self, int32_t index, dtrpc_handle* handle);

extern void
dtrpc_registry_dispose(dtrpc_registry_t* self);