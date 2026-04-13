/*
 * dtobject -- Vtable facade for basic object lifecycle and identity operations.
 *
 * Defines a seven-method dispatch interface covering creation, copy, disposal,
 * equality, class-name query, interface membership, and string rendering.
 * Implementations register a vtable keyed by model number; the facade
 * dispatches through opaque handles, enabling generic containers and utilities
 * to manage objects without knowing their concrete types.
 *
 * cdox v1.0.2
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <dtcore/dterr.h>

// opaque handle for dispatch calls
struct dtobject_handle_t;
typedef struct dtobject_handle_t* dtobject_handle;

// arguments
#define DTOBJECT_CREATE_ARGS
#define DTOBJECT_COPY_ARGS , void* that
#define DTOBJECT_DISPOSE_ARGS
#define DTOBJECT_EQUALS_ARGS , void* that
#define DTOBJECT_GET_CLASS_ARGS
#define DTOBJECT_IS_IFACE_ARGS , const char* iface_name
#define DTOBJECT_TO_STRING_ARGS , char *buffer, size_t buffer_size

#define DTOBJECT_CREATE_PARAMS
#define DTOBJECT_COPY_PARAMS , that
#define DTOBJECT_DISPOSE_PARAMS
#define DTOBJECT_EQUALS_PARAMS , that
#define DTOBJECT_GET_CLASS_PARAMS
#define DTOBJECT_IS_IFACE_PARAMS , iface_name
#define DTOBJECT_TO_STRING_PARAMS , buffer, buffer_size

// delegates
typedef dterr_t* (*dtobject_create_fn)(void* handle DTOBJECT_CREATE_ARGS);
typedef void (*dtobject_copy_fn)(void* self DTOBJECT_COPY_ARGS);
typedef void (*dtobject_dispose_fn)(void* self DTOBJECT_DISPOSE_ARGS);
typedef bool (*dtobject_equals_fn)(void* self DTOBJECT_EQUALS_ARGS);
typedef const char* (*dtobject_get_class_fn)(void* self DTOBJECT_GET_CLASS_ARGS);
typedef bool (*dtobject_is_iface_fn)(void* self DTOBJECT_IS_IFACE_ARGS);
typedef void (*dtobject_to_string_fn)(void* self DTOBJECT_TO_STRING_ARGS);

// virtual table type
typedef struct dtobject_vt_t
{
    dtobject_create_fn create;
    dtobject_copy_fn copy;
    dtobject_dispose_fn dispose;
    dtobject_equals_fn equals;
    dtobject_get_class_fn get_class;
    dtobject_is_iface_fn is_iface;
    dtobject_to_string_fn to_string;
} dtobject_vt_t;

// vtable registration
extern dterr_t*
dtobject_set_vtable(int32_t model_number, dtobject_vt_t* vtable);
extern dterr_t*
dtobject_get_vtable(int32_t model_number, dtobject_vt_t** vtable);

// declaration dispatcher or implementation
#define DTOBJECT_DECLARE_API_EX(NAME, T)                                                                                       \
    extern void NAME##_copy(NAME##T self, NAME##T that);                                                                       \
    extern void NAME##_dispose(NAME##T self DTOBJECT_DISPOSE_ARGS);                                                            \
    extern bool NAME##_equals(NAME##T self, NAME##T that);                                                                     \
    extern const char* NAME##_get_class(NAME##T self DTOBJECT_GET_CLASS_ARGS);                                                 \
    extern bool NAME##_is_iface(NAME##T self DTOBJECT_IS_IFACE_ARGS);                                                          \
    extern void NAME##_to_string(NAME##T self DTOBJECT_TO_STRING_ARGS);

// declare dispatcher
extern dterr_t*
dtobject_create(int32_t model_number, dtobject_handle* handle);
DTOBJECT_DECLARE_API_EX(dtobject, _handle)

// declare implementation (put this in its .h file)
#define DTOBJECT_DECLARE_API(NAME)                                                                                             \
    extern dterr_t* NAME##_create(NAME##_t** self);                                                                            \
    DTOBJECT_DECLARE_API_EX(NAME, _t*)

// initialize implementation vtable (put this in its .c file)
#define DTOBJECT_INIT_VTABLE(NAME)                                                                                             \
    static dtobject_vt_t NAME##_object_vt = {                                                                                  \
        .create = (dtobject_create_fn)NAME##_create,                                                                           \
        .copy = (dtobject_copy_fn)NAME##_copy,                                                                                 \
        .dispose = (dtobject_dispose_fn)NAME##_dispose,                                                                        \
        .equals = (dtobject_equals_fn)NAME##_equals,                                                                           \
        .get_class = (dtobject_get_class_fn)NAME##_get_class,                                                                  \
        .is_iface = (dtobject_is_iface_fn)NAME##_is_iface,                                                                     \
        .to_string = (dtobject_to_string_fn)NAME##_to_string,                                                                  \
    };

// common members expected at the start of all implementation structures
#define DTOBJECT_COMMON_MEMBERS int32_t model_number;

#define DTOBJECT_MODEL_NUMBER(handle) (*((int32_t*)(handle)))

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtobject

This is a facade layer for basic object operations such as creation, copying, disposal and string rendering.

Many diverse objects in the system implement this facade to provide a common interface for basic object management.
It is espeicially useful for generic containers and utilities that need to manipulate objects without knowing their concrete types.

This facade is vtable-dispatched. Please refer to [dtvtable](dtvtable.md) for more information on vtable dispatching.


## Mini-guide

- For a class to implement this facade, it must register a `dtobject_vt_t` vtable for its model number and implement all the functions below.
- To call the facade functions, pass the handle to the function cast as `dtobject_handle`.

## Example Creation From Model Number

```c
#include <dtcore/dtobject.h>

my_object_handle h = NULL;
dtobject_create(MODEL_NUMBER, (dtobject_handle_t*)&h);
dtobject_to_string((dtobject_handle_t)h, buffer, buffer_size);
dtobject_dispose((dtobject_handle_t)h);

```

## Example Handle Sharing
```c
#include <dtcore/dtobject.h>
#include <dtcore/dtrandomizer_uniform.h>

dtrandomizer_handle r = NULL;
// create a dtrandomizer_uniform object, which we know implements dtobject facade
dtrandomizer_uniform_create(&r);
// use dtobject facade to render it to string
dtobject_to_string((dtobject_handle_t)r, buffer, buffer_size);

```

## Example Implementation

```c
#include <dtcore/dtobject.h>

#define MODEL_NUMBER 1

static dterr_t* my_create(void* handle) { (void)handle; return NULL; }
static void my_copy(void* self, void* that) { (void)self; (void)that; }
static void my_dispose(void* self) { (void)self; }
static bool my_equals(void* self, void* that) { (void)self; (void)that; return true; }
static const char* my_get_class(void* self) { (void)self; return "my_t"; }
static bool my_is_iface(void* self, const char* iface_name) { (void)self; (void)iface_name; return false; }
static void my_to_string(void* self, char* buffer, size_t buffer_size) { (void)self; (void)buffer; (void)buffer_size; }

int main(void)
{
    dtobject_vt_t vt = {
        .create = my_create,
        .copy = my_copy,
        .dispose = my_dispose,
        .equals = my_equals,
        .get_class = my_get_class,
        .is_iface = my_is_iface,
        .to_string = my_to_string,
    };

    (void)dtobject_set_vtable(MODEL_NUMBER, &vt);

    dtobject_handle h = NULL;
    (void)dtobject_create(MODEL_NUMBER, &h);
    dtobject_dispose(h);

    return 0;
}
```

## Data structures

### dtobject_handle

Defines an opaque handle used for dispatch calls.

### dtobject_vt_t

Defines the vtable used to dispatch object operations.

Members:

> `dtobject_create_fn create` Create delegate for the model.  
> `dtobject_copy_fn copy` Copy delegate for the model.  
> `dtobject_dispose_fn dispose` Dispose delegate for the model.  
> `dtobject_equals_fn equals` Equality delegate for the model.  
> `dtobject_get_class_fn get_class` Class-name delegate for the model.  
> `dtobject_is_iface_fn is_iface` Interface test delegate for the model.  
> `dtobject_to_string_fn to_string` String rendering delegate for the model.  

## Macros

### DTOBJECT_COMMON_MEMBERS

Defines common members required at the start of implementation structures.

### DTOBJECT_DECLARE_API

`DTOBJECT_DECLARE_API(NAME)`

Declares a typed object API that maps to the common dispatch surface.

### DTOBJECT_INIT_VTABLE

`DTOBJECT_INIT_VTABLE(NAME)`

Defines a vtable initializer that binds a typed API to the common vtable shape.

## Functions

### dtobject_copy

Dispatches a copy operation through the vtable for the handle model number.

Params:

> `dtobject_handle handle` Object handle to copy into.  
> `dtobject_handle that` Object handle to copy from.  

Return: `void`  No return value.  

### dtobject_create

Dispatches a create operation through the vtable for a model number.

Params:

> `int32_t model_number` Model number used to select a registered vtable.  
> `dtobject_handle* handle` Output location for the created handle.  

Return: `dterr_t*`  Error on failure, or NULL on success.  

### dtobject_dispose

Dispatches a dispose operation through the vtable for the handle model number.

Params:

> `dtobject_handle handle` Object handle to dispose.  

Return: `void`  No return value.  

### dtobject_equals

Dispatches an equality operation through the vtable for the handle model number.

Params:

> `dtobject_handle handle` First object handle to compare.  
> `dtobject_handle that` Second object handle to compare.  

Return: `bool`  True if equal, otherwise false.  

### dtobject_get_class

Dispatches a class-name query through the vtable for the handle model number.

Params:

> `dtobject_handle handle` Object handle to query.  

Return: `const char*`  Class name string, or NULL on failure.  

### dtobject_get_vtable

Retrieves a registered vtable for a model number.

Params:

> `int32_t model_number` Model number used to select a registered vtable.  
> `dtobject_vt_t** vtable` Output location for the vtable pointer.  

Return: `dterr_t*`  Error on failure, or NULL on success.  

### dtobject_is_iface

Dispatches an interface-membership query through the vtable for the handle model number.

Params:

> `dtobject_handle handle` Object handle to query.  
> `const char* iface_name` Interface name to test.  

Return: `bool`  True if the interface is supported, otherwise false.  

### dtobject_set_vtable

Registers a vtable for a model number in the dispatch registry.

Params:

> `int32_t model_number` Model number used to identify the vtable.  
> `dtobject_vt_t* vtable` Vtable to register for dispatch.  

Return: `dterr_t*`  Error on failure, or NULL on success.  

### dtobject_to_string

Dispatches a string rendering operation through the vtable for the handle model number.

Params:

> `dtobject_handle handle` Object handle to render.  
> `char *buffer` Output buffer for the rendered string.  
> `size_t buffer_size` Size of the output buffer in bytes.  

Return: `void`  No return value.  

<!-- FG_IDC: 82548800-24bd-4419-8f25-e6bd6dee801a | FG_UTC: 2026-01-17T13:14:03Z | FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
