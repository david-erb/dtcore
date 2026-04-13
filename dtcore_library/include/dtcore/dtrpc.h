/*
 * dtrpc -- Vtable facade for KVP-based remote procedure calls.
 *
 * Defines a single-method dispatch interface where request and response
 * are both dtkvp_list_t instances.  A refusal flag allows the callee to
 * decline without treating the exchange as an error.  Implementations
 * register a vtable keyed by model number for selection at runtime.
 *
 * cdox v1.0.2
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <dtcore/dterr.h>
#include <dtcore/dtkvp.h>

// opaque handle for dispatch calls
struct dtrpc_handle_t;
typedef struct dtrpc_handle_t* dtrpc_handle;

// arguments
#define DTRPC_CALL_ARGS , dtkvp_list_t *request_kvp_list, bool *was_refused, dtkvp_list_t *response_kvp_list
#define DTRPC_CALL_PARAMS , request_kvp_list, was_refused, response_kvp_list

// delegates
typedef dterr_t* (*dtrpc_call_fn)(void* self DTRPC_CALL_ARGS);
typedef void (*dtrpc_dispose_fn)(void* self);

// virtual table type
typedef struct dtrpc_vt_t
{
    dtrpc_call_fn call;
    dtrpc_dispose_fn dispose;
} dtrpc_vt_t;

// vtable registration
extern dterr_t*
dtrpc_set_vtable(int32_t model_number, dtrpc_vt_t* vtable);

extern dterr_t*
dtrpc_get_vtable(int32_t model_number, dtrpc_vt_t** vtable);

// declaration dispatcher or implementation
#define DTRPC_DECLARE_API_EX(NAME, T)                                                                                          \
    extern dterr_t* NAME##_call(NAME##T self DTRPC_CALL_ARGS);                                                                 \
    extern void NAME##_dispose(NAME##T self);

// declare dispatcher
DTRPC_DECLARE_API_EX(dtrpc, _handle)

// declare implementation (put this in its .h file)
#define DTRPC_DECLARE_API(NAME) DTRPC_DECLARE_API_EX(NAME, _t*)

// initialize implementation vtable (put this in its .c file)
#define DTRPC_INIT_VTABLE(NAME)                                                                                                \
    static dtrpc_vt_t NAME##_vt = {                                                                                            \
        .call = (dtrpc_call_fn)NAME##_call,                                                                                    \
        .dispose = (dtrpc_dispose_fn)NAME##_dispose,                                                                           \
    };

// common members expected at the start of all implementation structures
#define DTRPC_COMMON_MEMBERS int32_t model_number;