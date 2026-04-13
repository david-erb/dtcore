/*
 * dttrap -- Conditional diagnostic accumulator with count-triggered callback.
 *
 * Accumulates formatted text lines and increments a counter on each event.
 * Once the count reaches a configured threshold, trapping stops and an
 * optional callback fires.  Intended for capturing the first N occurrences
 * of a repeating condition during debugging.
 *
 * cdox v1.0.2
 */
#pragma once

#include <stdint.h>

#include <dtcore/dterr.h>
#include <dtcore/dtlog.h>
#include <dtcore/dtstr.h>

typedef struct dttrap_t dttrap_t;

// call back for when count is reached
typedef dterr_t* (*dttrap_callback_fn)(dttrap_t* self, void* user_context);

typedef struct dttrap_t
{
  int32_t stop_trapping_after_count;
  int32_t count;
  char* lines;
  dttrap_callback_fn callback;
  void* callback_user_context;
} dttrap_t;

// -------------------------------------------------------------------------------
extern dterr_t*
dttrap_init(dttrap_t* self);
extern dterr_t*
dttrap_count(dttrap_t* self);
extern dterr_t*
dttrap_append(dttrap_t* self, const char* format, ...);
extern dterr_t*
dttrap_debug(dttrap_t* self);
extern dterr_t*
dttrap_set_callback(dttrap_t* self,
                    dttrap_callback_fn callback,
                    void* user_context);
extern void
dttrap_dispose(dttrap_t* self);

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dttrap

Accumulate diagnostic lines until a trigger threshold.

This group of functions provides conditional accumulation of formatted diagnostic output for a single instance. 
It is useful for debugging paths where repeated events should be captured and examined after a fixed number of occurrences. 

## Mini-guide

- Initialize the instance before use by calling `dttrap_init` to set defaults and clear state.
- Increment the internal counter by calling `dttrap_count` at the event site to track occurrences.
- Append formatted text by calling `dttrap_append` while the count is below the stop threshold.
- Register a callback with `dttrap_set_callback` to be notified when the threshold is reached.
- Release owned resources by calling `dttrap_dispose` when the instance is no longer needed.

## Example

```c
dttrap_t trap;
dttrap_init(&trap);
dttrap_set_callback(&trap, on_trap_reached, user_ctx);

for (int i = 0; i < 12; ++i)
{
    dttrap_count(&trap);
    dttrap_append(&trap, "iteration %d", i);
}

dttrap_debug(&trap);
dttrap_dispose(&trap);
```

## Data structures

### dttrap_callback_fn

Defines the signature for a callback invoked when the count threshold is reached.

Members:

> `dterr_t* (*)(dttrap_t* self, void* user_context)`  

### dttrap_t

Holds trapping state, accumulated lines, and callback configuration.

Members:

> `int32_t stop_trapping_after_count` Count at which trapping stops and the callback may be invoked.  
> `int32_t count` Current accumulated count.  
> `char* lines` Accumulated formatted text lines.  
> `dttrap_callback_fn callback` Callback invoked when the threshold is reached.  
> `void* callback_user_context` User-provided context passed to the callback.  

## Functions

### dttrap_append

Appends a formatted line to the internal buffer while trapping is active.

Params:

> `dttrap_t* self` Trap instance to append to.  
> `const char* format` printf-style format string.  
> `...` Format arguments.  

Return: `dterr_t*` Error information, or NULL on success or when trapping is inactive.

### dttrap_count

Increments the internal count and checks for threshold completion.

Params:

> `dttrap_t* self` Trap instance to update.  

Return: `dterr_t*` Error information, or NULL on success.

### dttrap_debug

Emits the accumulated lines using the debug logging facility.

Params:

> `dttrap_t* self` Trap instance whose lines are logged.  

Return: `dterr_t*` Error information, or NULL on success.

### dttrap_dispose

Releases resources owned by the trap and clears its state.

Param: `void`  No parameters.

Return: `void`  No return value.

### dttrap_init

Initializes a trap instance to a known default state.

Params:

> `dttrap_t* self` Trap instance to initialize.  

Return: `dterr_t*` Error information, or NULL on success.

### dttrap_set_callback

Registers a callback to be invoked when the count threshold is reached.

Params:

> `dttrap_t* self` Trap instance to configure.  
> `dttrap_callback_fn callback` Callback function to invoke.  
> `void* user_context` User context passed to the callback.  

Return: `dterr_t*` Error information, or NULL on success.

<!-- FG_IDC: fa9e2f7c-a510-43a6-9850-6c736ec52326 | FG_UTC: 2026-01-18T09:15:10Z | FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
