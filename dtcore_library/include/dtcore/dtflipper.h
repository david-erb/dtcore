#pragma once
// See markdown documentation at the end of this file.

#include <dtcore/dterr.h>

typedef struct dtflipper_t
{
  void* upper_buffer;
  void* lower_buffer;
  volatile int upper_busy;
  volatile int lower_busy;
} dtflipper_t;

typedef enum
{
  OK,
  UPPER_BUSY,
  LOWER_BUSY
} dtflipper_flip_result_e;

extern dterr_t*
dtflipper_init(dtflipper_t* self, void* upper_buffer, void* lower_buffer);
extern dtflipper_flip_result_e
dtflipper_flip_critical(dtflipper_t* self);
extern dterr_t*
dtflipper_flip(dtflipper_t* self);
extern dterr_t*
dtflipper_control_upper(dtflipper_t* self, void** payload);
extern dterr_t*
dtflipper_release_upper(dtflipper_t* self);
extern dterr_t*
dtflipper_control_lower(dtflipper_t* self, void** payload);
extern dterr_t*
dtflipper_release_lower(dtflipper_t* self);

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtflipper

Dual-buffer ownership flipping with busy-state coordination.

This group of functions provides buffer role flipping for paired memory buffers. 
It is intended for producer-consumer style handoff where only one side may access a buffer at a time.
It is designed to coordinate ownership without locks or allocation.
The implementation favors atomic/volatile busy flags over explicit locking for low-overhead concurrency control.

## Mini-guide

- Initialize the module by providing two preallocated buffers via the init function.
- Take exclusive access to a buffer by calling the corresponding control function before use.
- Release a buffer explicitly after use by calling the matching release function.
- Flip buffer roles only when neither buffer is marked busy.
- Use the critical flip variant inside a critical section to avoid system calls such as memory allocation for error reporting.

## Example

```c
dtflipper_t flipper;
void* upper;
void* lower;

dtflipper_init(&flipper, buffer_a, buffer_b);

dtflipper_control_upper(&flipper, &upper);
/* use upper buffer */
dtflipper_release_upper(&flipper);

dtflipper_flip(&flipper);

dtflipper_control_lower(&flipper, &lower);
/* use lower buffer */
dtflipper_release_lower(&flipper);
```

## Data structures

### dtflipper_flip_result_e

Represents the result of a buffer flip attempt.

Members:

> `OK` Indicates that the flip succeeded.  

> `UPPER_BUSY` Indicates that the upper buffer was busy.  

> `LOWER_BUSY` Indicates that the lower buffer was busy.  

### dtflipper_t

Holds buffer pointers and busy-state flags for flipping control.

Members:

> `void* upper_buffer` Pointer to the current upper buffer.  

> `void* lower_buffer` Pointer to the current lower buffer.  

> `volatile int upper_busy` Busy flag for the upper buffer.  

> `volatile int lower_busy` Busy flag for the lower buffer.  

## Functions

### dtflipper_control_lower

Grants exclusive access to the lower buffer.

Params:

> `dtflipper_t* self` Module instance.  

> `void** payload` Receives the lower buffer pointer.  

Return: `dterr_t*` Error object on failure or NULL on success.

### dtflipper_control_upper

Grants exclusive access to the upper buffer.

Params:

> `dtflipper_t* self` Module instance.  

> `void** payload` Receives the upper buffer pointer.  

Return: `dterr_t*` Error object on failure or NULL on success.

### dtflipper_flip

Attempts to swap upper and lower buffers with busy checks.

Params:

> `dtflipper_t* self` Module instance.  

Return: `dterr_t*` Error object on failure or NULL on success.

### dtflipper_flip_critical

Attempts to swap buffers without accessing system resources.

Params:

> `dtflipper_t* self` Module instance.  

Return: `dtflipper_flip_result_e` Result indicating success or busy state.

### dtflipper_init

Initializes the flipper with two buffers and clears busy flags.

Params:

> `dtflipper_t* self` Module instance.  

> `void* upper_buffer` Initial upper buffer pointer.  

> `void* lower_buffer` Initial lower buffer pointer.  

Return: `dterr_t*` Always NULL.

### dtflipper_release_lower

Releases exclusive access to the lower buffer.

Params:

> `dtflipper_t* self` Module instance.  

Return: `dterr_t*` Always NULL.

### dtflipper_release_upper

Releases exclusive access to the upper buffer.

Params:

> `dtflipper_t* self` Module instance.  

Return: `dterr_t*` Always NULL.

<!-- FG_IDC: cebda47e-e6c1-4620-8596-858533559e90 | FG_UTC: 2026-01-17T12:47:34Z FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
