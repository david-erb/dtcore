/*
 * dtvtable -- Model-indexed vtable registry for runtime dispatch.
 *
 * Maps 32-bit model numbers to vtable pointers using caller-provided
 * parallel arrays.  Registration is idempotent for existing mappings.
 * Lookup uses a linear scan up to the first empty slot.  Macros reduce
 * the boilerplate of defining and dispatching vtable-based APIs.
 *
 * cdox v1.0.2
 */
#pragma once

// See markdown documentation at the end of this file.

// Lightweight registry mapping 32-bit model numbers to vtable pointers.

#include <stdint.h>

#include <dtcore/dterr.h>

// Backing storage for the model→vtable registry.
// Note: The registry does **not** take ownership of vtable pointers; they are borrowed.
// Ensure each published vtable remains valid for the duration of its use.
//       Ensure each published vtable remains valid for the duration of its use.

typedef struct
{
    // Parallel array of length ::dtvtable_registry_t::max_vtables; `0` means empty slot.
    int32_t* model_numbers;
    // Parallel array of vtable pointers; valid when the corresponding model number is non-zero.
    void** vtables;
    // Number of usable entries (capacity > 0). Arrays must have this length.
    int max_vtables;
} dtvtable_registry_t;

// --------------------------------------------------------------------------------------------

// Register (publish) @p model_number → @p vtable in @p reg.
//
// Inserts into the first empty slot (where ::dtvtable_registry_t::model_numbers[i] is `0`).
// If @p model_number is already present, this call is **idempotent** and returns success
// without modifying the existing mapping.
//
// Warning: Not thread-safe; external synchronization is required in concurrent contexts.
extern dterr_t*
dtvtable_set(dtvtable_registry_t* reg, int32_t model_number, void* vtable);

// Retrieve the vtable pointer for @p model_number from @p reg.
//
// Scans until a match is found or the first empty slot is reached (compact-array layout).
// On success, if @p vtable is non-NULL, the found pointer is written to `*vtable`.
//
// Note: The returned pointer (via @p vtable) is borrowed; the registry does not own it.

extern dterr_t*
dtvtable_get(dtvtable_registry_t* reg, int32_t model_number, void** vtable);

// --------------------------------------------------------------------------------------------
// convenience macro to define vtable-dispatching functions for a class
#define DTVTABLE_DISPATCH(CLASS, NAME, ARGS, PARAMS, RET)                                                                      \
    RET CLASS##_##NAME(CLASS##_handle handle ARGS)                                                                             \
    {                                                                                                                          \
        if (!handle)                                                                                                           \
            return dterr_new(DTERR_FAIL, DTERR_LOC, NULL, #CLASS " handle is NULL");                                           \
        int32_t model = *((int32_t*)handle);                                                                                   \
        CLASS##_vt_t* vt = NULL;                                                                                               \
        dterr_t* dterr = CLASS##_get_vtable(model, &vt);                                                                       \
        if (dterr)                                                                                                             \
            return dterr;                                                                                                      \
        return vt->NAME((void*)handle PARAMS);                                                                                 \
    }

#define DTVTABLE_GET_FUNCTION(HANDLE, CLASS, FUNCTION, FN_VAR)                                                                 \
    {                                                                                                                          \
        int32_t model_number = ((int32_t*)HANDLE)[0];                                                                          \
        CLASS##_vt_t* vtable = NULL;                                                                                           \
        DTERR_C(CLASS##_get_vtable(model_number, &vtable));                                                                    \
        FN_VAR = vtable->FUNCTION;                                                                                             \
        if (FN_VAR == NULL)                                                                                                    \
        {                                                                                                                      \
            dterr = dterr_new(DTERR_BADARG, DTERR_LOC, NULL, "%s is NULL", #CLASS "_" #FUNCTION);                              \
            goto cleanup;                                                                                                      \
        }                                                                                                                      \
    }

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtvtable

Lightweight registry for model-indexed vtable dispatch

This group of functions provides a small registry for mapping 32-bit model numbers to vtable pointers for dispatch. 
It is intended for systems that select behavior by model identifier at runtime without dynamic allocation. 
It is designed to keep ownership external and make registration explicit. 
The implementation favors simple linear scans over additional indexing structures.

## Mini-guide

- Initialize the registry with parallel arrays sized by a fixed maximum capacity to define storage up front.
- Publish a mapping by calling the registration function, which inserts into the first empty slot.
- Re-registering an existing model with the same pointer succeeds without modifying the registry.
- Avoid concurrent access or add external synchronization because operations are not thread-safe.
- Treat all returned vtable pointers as borrowed and keep them valid for the required lifetime.

## Example

```c
#include <dtcore/dtvtable.h>

static int32_t models[8] = {0};
static void* vtables[8] = {0};

void example(void)
{
    dtvtable_registry_t reg = {
        .model_numbers = models,
        .vtables = vtables,
        .max_vtables = 8,
    };

    void* some_vtable = (void*)0x1234;

    dterr_t* err = dtvtable_set(&reg, 42, some_vtable);
    if (err)
        return;

    void* found = NULL;
    err = dtvtable_get(&reg, 42, &found);
    if (err)
        return;
}
```

## Data structures

### dtvtable_registry_t

Holds the backing storage and capacity for the model-to-vtable registry.

Members:

> `int32_t* model_numbers` Parallel array of model identifiers where zero indicates an empty slot.  
> 
> `void** vtables` Parallel array of vtable pointers aligned with the model number array.  
> 
> `int max_vtables` Number of usable entries defining the registry capacity.  

## Macros

### DTVTABLE_DISPATCH

`DTVTABLE_DISPATCH(CLASS, NAME, ARGS, PARAMS, RET)`

Defines a forwarding function that resolves a vtable by model number and dispatches a method call.

## Functions

### dtvtable_get

Looks up a vtable pointer associated with a model number in the registry.

Params:

> `dtvtable_registry_t* reg` Registry to search.  
> 
> `int32_t model_number` Model identifier to resolve.  
> 
> `void** vtable` Output location for the found vtable pointer.  

Return: `dterr_t*` NULL on success or an error describing failure.

### dtvtable_set

Registers a model number to vtable pointer mapping in the registry.

Params:

> `dtvtable_registry_t* reg` Registry to modify.  
> 
> `int32_t model_number` Model identifier to publish.  
> 
> `void* vtable` Vtable pointer to associate with the model.  

Return: `dterr_t*` NULL on success or an error describing failure.

<!-- FG_IDC: f0949ec5-a1e1-442f-a27c-2409336b8c87 | FG_UTC: 2026-01-17T10:22:51Z FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
