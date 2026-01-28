#pragma once

#include <stdint.h>

#include <dtcore/dterr.h>
#include <dtcore/dtobject.h>

// opaque handle for dispatch calls
struct dtrandomizer_handle_t;
typedef struct dtrandomizer_handle_t* dtrandomizer_handle;

// arguments
#define DTRANDOMIZER_NEXT_ARGS , int32_t* value
#define DTRANDOMIZER_RESET_ARGS
#define DTRANDOMIZER_NEXT_PARAMS , value
#define DTRANDOMIZER_RESET_PARAMS

// delegates
typedef dterr_t* (*dtrandomizer_next_fn)(void* self DTRANDOMIZER_NEXT_ARGS);
typedef dterr_t* (*dtrandomizer_reset_fn)(void* self DTRANDOMIZER_RESET_ARGS);
typedef void (*dtrandomizer_dispose_fn)(void* self);

// virtual table type
typedef struct dtrandomizer_vt_t
{
    dtrandomizer_next_fn next;
    dtrandomizer_reset_fn reset;
    dtrandomizer_dispose_fn dispose;
} dtrandomizer_vt_t;

// vtable registration
extern dterr_t*
dtrandomizer_set_vtable(int32_t model_number, dtrandomizer_vt_t* vtable);
extern dterr_t*
dtrandomizer_get_vtable(int32_t model_number, dtrandomizer_vt_t** vtable);

// declaration dispatcher or implementation
#define DTRANDOMIZER_DECLARE_API_EX(NAME, T)                                                                                   \
    extern dterr_t* NAME##_next(NAME##T self DTRANDOMIZER_NEXT_ARGS);                                                          \
    extern dterr_t* NAME##_reset(NAME##T self DTRANDOMIZER_RESET_ARGS);                                                        \
    extern void NAME##_dispose(NAME##T self);

// declare dispatcher
DTRANDOMIZER_DECLARE_API_EX(dtrandomizer, _handle)

// declare implementation (put this in its .h file)
#define DTRANDOMIZER_DECLARE_API(NAME) DTRANDOMIZER_DECLARE_API_EX(NAME, _t*)

// initialize implementation vtable (put this in its .c file)
#define DTRANDOMIZER_INIT_VTABLE(NAME)                                                                                         \
    static dtrandomizer_vt_t NAME##_randomizer_vt = {                                                                          \
        .next = (dtrandomizer_next_fn)NAME##_next,                                                                             \
        .reset = (dtrandomizer_reset_fn)NAME##_reset,                                                                          \
        .dispose = (dtrandomizer_dispose_fn)NAME##_dispose,                                                                    \
    };

// common members expected at the start of all implementation structures
#define DTRANDOMIZER_COMMON_MEMBERS int32_t model_number;

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtrandomizer


Facade provides a uniform interface for producing sequences of random integers.

Please refer to [dtvtable](dtvtable.md) for more information on vtable dispatching.

You can use this facade to implement various randomization strategies, such as uniform distribution, Gaussian distribution, or custom algorithms.
The particular algorithm is selected at runtime based on the model number associated with the randomizer instance, 
allowing configurable behavior without changing the call sites.

## Mini-guide


- For a class to implement this facade, it must register a `dtrandomizer_vt_t` vtable for its model number and implement all the functions below.
- To call the facade functions, pass the handle to the function cast as `dtrandomizer_handle`.

## Example

```c
#include <dtcore/dtrandomizer.h>
#include <dtcore/dtrandomizer_uniform.h>

dtrandomizer_handle h = NULL;

// crate a concrete uniform randomizer instance as handle
dtrandomizer_uniform_create((dtrandomizer_uniform_t*)&h);

// access the randomizer via the dispatch API
int32_t v = 0;
dtrandomizer_next(h, &v);

dtrandomizer_dispose(h);

```

## Data structures

### dtrandomizer_handle

Defines the opaque handle type used by the dispatch API.

### dtrandomizer_vt_t

Provides the dispatch table for the randomizer interface.

Members
:

> `dtrandomizer_next_fn next` Produces the next value for the implementation.  
> `dtrandomizer_reset_fn reset` Resets the implementation state.  
> `dtrandomizer_dispose_fn dispose` Disposes the implementation instance.  

## Macros

### DTRANDOMIZER_COMMON_MEMBERS

Defines the common leading members required by implementations.

### DTRANDOMIZER_DECLARE_API

`DTRANDOMIZER_DECLARE_API(NAME)`

Declares the implementation-side randomizer interface functions for a concrete type.

### DTRANDOMIZER_INIT_VTABLE

`DTRANDOMIZER_INIT_VTABLE(NAME)`

Defines a static `dtrandomizer_vt_t` that binds the implementation interface functions.

## Functions

### dtrandomizer_dispose

Disposes a randomizer instance by dispatching to its registered vtable.

Params
:

> `dtrandomizer_handle handle` Randomizer instance handle used for dispatch.  

Return: `void`  No return value.  


### dtrandomizer_next

Produces the next randomizer value by dispatching to its registered vtable.

Params
:

> `dtrandomizer_handle handle` Randomizer instance handle used for dispatch.  
> `int32_t* value` Output location that receives the produced value.  

Return: `dterr_t*` Error pointer, or `NULL` on success.  

### dtrandomizer_reset

Resets a randomizer instance by dispatching to its registered vtable.

Params
:

> `dtrandomizer_handle handle` Randomizer instance handle used for dispatch.  

Return: `dterr_t*` Error pointer, or `NULL` on success.  


<!-- FG_IDC: 4a44bea7-e15f-465c-ba6d-fb1a59c42ead | FG_UTC: 2026-01-19T03:04:59Z FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
