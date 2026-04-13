/*
 * dtguidable -- Vtable facade for GUID retrieval.
 *
 * Defines a single-method dispatch interface allowing any object to expose
 * its GUID through a common handle without exposing its concrete type.
 * Implementations register a vtable keyed by model number; callers invoke
 * the dispatcher with an opaque handle.
 *
 * cdox v1.0.2
 */
#pragma once

#include <stdint.h>

#include <dtcore/dterr.h>
#include <dtcore/dtguid.h>

// opaque handle for dispatch calls
struct dtguidable_handle_t;
typedef struct dtguidable_handle_t* dtguidable_handle;

// delegates
typedef dterr_t* (*dtguidable_get_guid_fn)(void* self, dtguid_t* guid);

// virtual table type
typedef struct dtguidable_vt_t
{
    dtguidable_get_guid_fn get_guid;
} dtguidable_vt_t;

// vtable registration
extern dterr_t*
dtguidable_set_vtable(int32_t model_number, dtguidable_vt_t* vtable);
extern dterr_t*
dtguidable_get_vtable(int32_t model_number, dtguidable_vt_t** vtable);

// declaration dispatcher or implementation
#define DTGUIDABLE_DECLARE_API_EX(NAME, T) extern dterr_t* NAME##_get_guid(NAME##T self, dtguid_t* guid);

// declare dispatcher
DTGUIDABLE_DECLARE_API_EX(dtguidable, _handle)

// declare implementation (put this in its .h file)
#define DTGUIDABLE_DECLARE_API(NAME) DTGUIDABLE_DECLARE_API_EX(NAME, _t*)

// initialize implementation vtable (put this in its .c file)
#define DTGUIDABLE_INIT_VTABLE(NAME)                                                                                           \
    static dtguidable_vt_t NAME##_guidable_vt = { .get_guid = (dtguidable_get_guid_fn)NAME##_get_guid };

// common members expected at the start of all implementation structures
#define DTGUIDABLE_COMMON_MEMBERS int32_t model_number;

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtguidable

This is a facade layer which allows an object to expose its GUID.

This facade is vtable-dispatched. Please refer to [dtvtable](dtvtable.md) for more information on vtable dispatching.

Various objects in the system implement this facade to provide a common interface for retrieving their GUIDs. 
It is especially useful for transferring object identity information across processor network boundaries.

## Mini-guide

- For a class to implement this facade, it must register a `dtguidable_vt_t` vtable for its model number and implement the get_guid function.
- To call the facade function dtguidable_get_guid, pass the handle to the function cast as `dtguidable_handle`.
- Retrieve a GUID through the dispatcher function rather than calling implementation functions directly.

## Example

```c
dtguid_t guid;
my_obj_handle handle = NULL;
obtain_my_obj_handle_somehow(&handle);
dtguidable_get_guid((dtguidable_handle)handle, &guid);
```

## Data structures


### dtguidable_handle

Opaque handle used for dispatching GUID requests.


## Macros

### DTGUIDABLE_COMMON_MEMBERS

Defines required leading members for implementation structures.

### DTGUIDABLE_DECLARE_API

Declares the GUID retrieval implementation for a concrete type.


### DTGUIDABLE_INIT_VTABLE

Defines and initializes a static vtable for an implementation.

## Functions

### dtguidable_get_guid

Retrieves the GUID associated with a handle using its registered vtable.

Params:

> `const dtguidable_handle handle`  Opaque handle identifying the object.  
> `dtguid_t* guid`  Output structure to receive the GUID.  

Return: `dterr_t*`  Error object on failure or NULL on success.

<!-- FG_IDC: bedfae00-6e31-49d8-80eb-473434bfb7ef | FG_UTC: 2026-01-24T04:31:15Z FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
