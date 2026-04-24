/*
 * dtraster -- Vtable facade for pixel buffer operations.
 *
 * Defines a dispatch interface for raster blitting, per-pixel read/write,
 * buffer management, shape query, and compatible raster creation.  Concrete
 * implementations register a vtable keyed by model number, allowing
 * framebuffers, sprites, and format-converting rasters to be used
 * interchangeably.  Includes primitive filled and outlined rectangle drawing
 * built on top of the facade.
 *
 * cdox v1.0.2
 */
#pragma once

#include <stdint.h>

#include <dtcore/dtbuffer.h>
#include <dtcore/dterr.h>
#include <dtcore/dtobject.h>
#include <dtcore/dtrgba8888.h>

typedef struct dtraster_shape_t
{
    int32_t w;
    int32_t h;
    int32_t stride_bytes;
} dtraster_shape_t;

// opaque handle for dispatch calls
struct dtraster_handle_t;
typedef struct dtraster_handle_t* dtraster_handle;

// arguments
#define DTRASTER_BLIT_ARGS , dtraster_handle that_handle, int32_t x, int32_t y
#define DTRASTER_CREATE_COMPATIBLE_RASTER_ARGS , dtraster_handle *out_raster_handle, int32_t w, int32_t h
#define DTRASTER_USE_BUFFER_ARGS , dtbuffer_t* buffer
#define DTRASTER_NEW_BUFFER_ARGS
#define DTRASTER_FETCH_PIXEL_ARGS , int32_t x, int32_t y, dtrgba8888_t *rgba8888
#define DTRASTER_STORE_PIXEL_ARGS , int32_t x, int32_t y, dtrgba8888_t rgba8888
#define DTRASTER_GET_SHAPE_ARGS , dtraster_shape_t* shape

#define DTRASTER_BLIT_PARAMS , that_handle, x, y
#define DTRASTER_CREATE_COMPATIBLE_RASTER_PARAMS , out_raster_handle, w, h
#define DTRASTER_USE_BUFFER_PARAMS , buffer
#define DTRASTER_NEW_BUFFER_PARAMS
#define DTRASTER_FETCH_PIXEL_PARAMS , x, y, rgba8888
#define DTRASTER_STORE_PIXEL_PARAMS , x, y, rgba8888
#define DTRASTER_GET_SHAPE_PARAMS , shape

// delegates
typedef dterr_t* (*dtraster_blit_fn)(void* self DTRASTER_BLIT_ARGS);
typedef dterr_t* (*dtraster_create_compatible_raster_fn)(void* self DTRASTER_CREATE_COMPATIBLE_RASTER_ARGS);
typedef dterr_t* (*dtraster_use_buffer_fn)(void* self DTRASTER_USE_BUFFER_ARGS);
typedef dterr_t* (*dtraster_new_buffer_fn)(void* self DTRASTER_NEW_BUFFER_ARGS);
typedef dterr_t* (*dtraster_fetch_pixel_fn)(void* self DTRASTER_FETCH_PIXEL_ARGS);
typedef dterr_t* (*dtraster_store_pixel_fn)(void* self DTRASTER_STORE_PIXEL_ARGS);
typedef dterr_t* (*dtraster_get_shape_fn)(void* self DTRASTER_GET_SHAPE_ARGS);
typedef void (*dtraster_dispose_fn)(void* self);

// virtual table type
typedef struct dtraster_vt_t
{
    dtraster_blit_fn blit;
    dtraster_create_compatible_raster_fn create_compatible_raster;
    dtraster_use_buffer_fn use_buffer;
    dtraster_new_buffer_fn new_buffer;
    dtraster_fetch_pixel_fn fetch_pixel;
    dtraster_store_pixel_fn store_pixel;
    dtraster_get_shape_fn get_shape;
    dtraster_dispose_fn dispose;

} dtraster_vt_t;

// vtable registration
extern dterr_t*
dtraster_set_vtable(int32_t model_number, dtraster_vt_t* vtable);

extern dterr_t*
dtraster_get_vtable(int32_t model_number, dtraster_vt_t** vtable);

// declaration dispatcher or implementation
#define DTRASTER_DECLARE_API_EX(NAME, T)                                                                                       \
    extern dterr_t* NAME##_blit(NAME##T self DTRASTER_BLIT_ARGS);                                                              \
    extern dterr_t* NAME##_create_compatible_raster(NAME##T self DTRASTER_CREATE_COMPATIBLE_RASTER_ARGS);                      \
    extern dterr_t* NAME##_use_buffer(NAME##T self DTRASTER_USE_BUFFER_ARGS);                                                  \
    extern dterr_t* NAME##_new_buffer(NAME##T self DTRASTER_NEW_BUFFER_ARGS);                                                  \
    extern dterr_t* NAME##_fetch_pixel(NAME##T self DTRASTER_FETCH_PIXEL_ARGS);                                                \
    extern dterr_t* NAME##_store_pixel(NAME##T self DTRASTER_STORE_PIXEL_ARGS);                                                \
    extern dterr_t* NAME##_get_shape(NAME##T self DTRASTER_GET_SHAPE_ARGS);                                                    \
    extern void NAME##_dispose(NAME##T self);

// declare dispatcher
DTRASTER_DECLARE_API_EX(dtraster, _handle)

// declare implementation (put this in its .h file)
#define DTRASTER_DECLARE_API(NAME) DTRASTER_DECLARE_API_EX(NAME, _t*)

// initialize implementation vtable (put this in its .c file)
#define DTRASTER_INIT_VTABLE(NAME)                                                                                             \
    static dtraster_vt_t NAME##_raster_vt = {                                                                                  \
        .blit = (dtraster_blit_fn)NAME##_blit,                                                                                 \
        .create_compatible_raster = (dtraster_create_compatible_raster_fn)NAME##_create_compatible_raster,                     \
        .use_buffer = (dtraster_use_buffer_fn)NAME##_use_buffer,                                                               \
        .new_buffer = (dtraster_new_buffer_fn)NAME##_new_buffer,                                                               \
        .fetch_pixel = (dtraster_fetch_pixel_fn)NAME##_fetch_pixel,                                                            \
        .store_pixel = (dtraster_store_pixel_fn)NAME##_store_pixel,                                                            \
        .get_shape = (dtraster_get_shape_fn)NAME##_get_shape,                                                                  \
        .dispose = (dtraster_dispose_fn)NAME##_dispose,                                                                        \
    };

// common members expected at the start of all implementation structures
#define DTRASTER_COMMON_MEMBERS int32_t model_number;

// --------------------------------------------------------------------------------------------
dterr_t*
dtraster_draw_rectangle_fill(dtraster_handle raster_handle, int32_t x, int32_t y, int32_t w, int32_t h, dtrgba8888_t rgba8888);
dterr_t*
dtraster_draw_rectangle_edge(dtraster_handle raster_handle, int32_t x, int32_t y, int32_t w, int32_t h, dtrgba8888_t rgba8888);
dterr_t*
dtraster_draw_rectangles_primary(dtraster_handle raster_handle,
  int32_t x,
  int32_t y,
  int32_t w,
  int32_t h,
  int32_t gutter,
  bool vertially,
  dtrgba8888_t rgba8888_edge_color);

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtraster

Facade provides a uniform interface for raster pixel buffers.

Please refer to [dtvtable](dtvtable.md) for more information on vtable dispatching.

The raster facade allows multiple raster implementations to be used interchangeably.
Examples include RAM-backed framebuffers, sprite rasters, display-backed buffers, or format-converting rasters.

Each raster implementation provides the same basic operations for copying pixels and accessing individual pixel values.

## Mini-guide

- For a class to implement this facade, it must register a `dtraster_vt_t` vtable for its model number and implement all the functions below.
- To call the facade functions, pass the handle to the function cast as `dtraster_handle`.
- Rasters may represent framebuffers, sprite images, glyph buffers, or other pixel containers.
- Bulk operations such as raster-to-raster blitting should be preferred over per-pixel operations when possible for performance reasons.

## Example

```c
#include <dtcore/dtraster.h>
#include <dtcore/dtraster_ram.h>

dtraster_handle dst = NULL;
dtraster_handle src = NULL;

// create two raster instances
dtraster_ram_create((dtraster_ram_t*)&dst, 320, 240);
dtraster_ram_create((dtraster_ram_t*)&src, 32, 32);

// copy pixels from src into dst
dtraster_blit(dst, src);

// set a pixel
dtrgba8888_t red = dtrgba8888(255,0,0,255);
dtraster_store_pixel(dst, 10, 10, red);

// read a pixel
dtrgba8888_t c;
dtraster_fetch_pixel(dst, 10, 10, &c);

dtraster_dispose(src);
dtraster_dispose(dst);
```

## Data structures

### dtraster_handle

Defines the opaque handle type used by the dispatch API.

### dtraster_vt_t

Provides the dispatch table for the raster interface.

Members :

> `dtraster_blit_fn blit` Copies pixel data from one raster into
> another.\
> `dtraster_fetch_pixel_fn fetch_pixel` Retrieves a pixel color from the
> raster.\
> `dtraster_store_pixel_fn store_pixel` Writes a pixel color into the
> raster.\
> `dtraster_dispose_fn dispose` Disposes the raster instance.

## Macros

### DTRASTER_COMMON_MEMBERS

Defines the common leading members required by raster implementations.

### DTRASTER_DECLARE_API

`DTRASTER_DECLARE_API(NAME)`

Declares the implementation-side raster interface functions for a
concrete type.

### DTRASTER_INIT_VTABLE

`DTRASTER_INIT_VTABLE(NAME)`

Defines a static `dtraster_vt_t` that binds the implementation interface
functions.

## Functions

### dtraster_blit

Copies pixel data from one raster into another by dispatching to the
implementation.

Params :

> `dtraster_handle handle` Destination raster instance used for
> dispatch.\
> `dtraster_handle src` Source raster whose pixel data will be copied.

Return: `dterr_t*` Error pointer, or `NULL` on success.

### dtraster_fetch_pixel

Retrieves a pixel color from the raster.

Params :

> `dtraster_handle handle` Raster instance used for dispatch.\
> `int32_t x` Horizontal pixel coordinate.\
> `int32_t y` Vertical pixel coordinate.\
> `dtrgba8888_t* color` Output location that receives the pixel color.

Return: `dterr_t*` Error pointer, or `NULL` on success.

### dtraster_store_pixel

Writes a pixel color into the raster.

Params :

> `dtraster_handle handle` Raster instance used for dispatch.\
> `int32_t x` Horizontal pixel coordinate.\
> `int32_t y` Vertical pixel coordinate.\
> `dtrgba8888_t color` Pixel color to write.

Return: `dterr_t*` Error pointer, or `NULL` on success.

### dtraster_dispose

Disposes a raster instance by dispatching to its registered vtable.

Params :

> `dtraster_handle handle` Raster instance handle used for dispatch.

Return: `void` No return value.

```

<!-- FG_IDC: 83f85c8d-1a7e-4e64-b15c-62b3b7f5a1f3 | FG_UTC: 2026-03-12T00:00:00Z FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
