#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <dtcore/dterr.h>
#include <dtcore/dtpackx.h>

// opaque handle for dispatch calls
struct dtpackable_handle_t;
typedef struct dtpackable_handle_t* dtpackable_handle;

// arguments
#define DTPACKABLE_PACKX_LENGTH_ARGS , int32_t* length
#define DTPACKABLE_PACKX_ARGS , uint8_t *output, int32_t *offset, int32_t length
#define DTPACKABLE_UNPACKX_ARGS , const uint8_t *input, int32_t *offset, int32_t length
#define DTPACKABLE_PACKX_LENGTH_PARAMS , length
#define DTPACKABLE_PACKX_PARAMS , output, offset, length
#define DTPACKABLE_UNPACKX_PARAMS , input, offset, length

// delegates
typedef dterr_t* (*dtpackable_packx_length_fn)(const void* self DTPACKABLE_PACKX_LENGTH_ARGS);
typedef dterr_t* (*dtpackable_packx_fn)(const void* self DTPACKABLE_PACKX_ARGS);
typedef dterr_t* (*dtpackable_unpackx_fn)(void* self DTPACKABLE_UNPACKX_ARGS);
typedef dterr_t* (*dtpackable_validate_unpacked_fn)(const void* self);

// virtual table type
typedef struct dtpackable_vt_t
{
    dtpackable_packx_length_fn packx_length;
    dtpackable_packx_fn packx;
    dtpackable_unpackx_fn unpackx;
    dtpackable_validate_unpacked_fn validate_unpacked;
} dtpackable_vt_t;

// vtable registration
extern dterr_t*
dtpackable_set_vtable(int32_t model_number, dtpackable_vt_t* vtable);
extern dterr_t*
dtpackable_get_vtable(int32_t model_number, dtpackable_vt_t** vtable);

// declaration dispatcher or implementation
#define DTPACKABLE_DECLARE_API_EX(NAME, T)                                                                                     \
    extern dterr_t* NAME##_packx_length(NAME##T self DTPACKABLE_PACKX_LENGTH_ARGS);                                            \
    extern dterr_t* NAME##_packx(NAME##T self DTPACKABLE_PACKX_ARGS);                                                          \
    extern dterr_t* NAME##_unpackx(NAME##T self DTPACKABLE_UNPACKX_ARGS);                                                      \
    extern dterr_t* NAME##_validate_unpacked(NAME##T self);

// declare dispatcher
DTPACKABLE_DECLARE_API_EX(dtpackable, _handle)

// declare implementation (put this in its .h file)
#define DTPACKABLE_DECLARE_API(NAME) DTPACKABLE_DECLARE_API_EX(NAME, _t*)

// initialize implementation vtable (put this in its .c file)
#define DTPACKABLE_INIT_VTABLE(NAME)                                                                                           \
    static dtpackable_vt_t NAME##_packable_vt = {                                                                              \
        .packx_length = (dtpackable_packx_length_fn)NAME##_packx_length,                                                       \
        .packx = (dtpackable_packx_fn)NAME##_packx,                                                                            \
        .unpackx = (dtpackable_unpackx_fn)NAME##_unpackx,                                                                      \
        .validate_unpacked = (dtpackable_validate_unpacked_fn)NAME##_validate_unpacked,                                        \
    };

// common members expected at the start of all implementation structures
#define DTPACKABLE_COMMON_MEMBERS int32_t model_number;

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtpackable

Facade for object serialization/deserialization via packing and unpacking binary data.

Please refer to [dtvtable](dtvtable.md) for more information on vtable dispatching.

The design pattern is for polymorphic objects that can serialize and deserialize their state
to and from binary representations, enabling storage and transmission.

## Mini-guide

- For a class to implement this facade, it must register a `dtpackable_vt_t` vtable for its model number and implement the pack and unpack functions.
- To call the facade functions, pass the handle to the function cast as `dtpackable_handle`.
- Use the length query before packing to size buffers correctly.  

## Example

```c
my_object_handle handle = obtain_your_object_handle_somehow();

// get the expected packed length
int32_t length = 0;
dterr_t* err = dtpackable_packx_length((dtpackable_handle)handle, &length);
if (err == NULL) {
    uint8_t* buffer = allocate_buffer(length);

    // pack the object into the buffer
    int32_t offset = 0;
    err = dtpackable_packx((dtpackable_handle)handle, buffer, &offset, length);
}
```

## Data structures

### dtpackable_handle

Opaque handle used for dispatch operations.

### dtpackable_vt_t

Virtual table defining pack and unpack operations.

Members:

> `dtpackable_packx_length_fn packx_length` Computes required packed length.  
> `dtpackable_packx_fn packx` Packs the object into a buffer.  
> `dtpackable_unpackx_fn unpackx` Unpacks the object from a buffer.  
> `dtpackable_validate_unpacked_fn validate_unpacked` Validates unpacked state.  

## Macros

### DTPACKABLE_COMMON_MEMBERS

Defines common members required at the start of implementation structures.

### DTPACKABLE_DECLARE_API

Declares the public API for a concrete implementation.

### DTPACKABLE_INIT_VTABLE

Initializes a static vtable for a concrete implementation.


## Functions

### dtpackable_packx

Dispatches a pack operation using the handle model number.

Params:

> `dtpackable_handle handle` Opaque handle identifying the object.  
> `uint8_t* output` Output buffer for packed data.  
> `int32_t* offset` In-out offset into the buffer.  
> `int32_t length` Total buffer length.  

Return: `dterr_t*` Error object on failure, or null on success.

### dtpackable_packx_length

Dispatches a packed length query using the handle model number.

Params:

> `dtpackable_handle handle` Opaque handle identifying the object.  
> `int32_t* length` Receives the required packed length.  

Return: `dterr_t*` Error object on failure, or null on success.

### dtpackable_unpackx

Dispatches an unpack operation using the handle model number.

Params:

> `dtpackable_handle handle` Opaque handle identifying the object.  
> `const uint8_t* input` Input buffer containing packed data.  
> `int32_t* offset` In-out offset into the buffer.  
> `int32_t length` Total buffer length.  

Return: `dterr_t*` Error object on failure, or null on success.

### dtpackable_validate_unpacked

Dispatches validation of unpacked state using the handle model number.

Params:

> `dtpackable_handle handle` Opaque handle identifying the object.  

Return: `dterr_t*` Error object on failure, or null on success.

<!-- FG_IDC: 4d4dbc9e-d5b1-478e-a762-7f0c7bb4fb73 | FG_UTC: 2026-01-18T15:04:42Z FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
