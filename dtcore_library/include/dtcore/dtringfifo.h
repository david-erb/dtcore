/*
 * dtringfifo -- Lock-free SPSC byte ring buffer over caller-provided storage.
 *
 * Implements single-producer single-consumer FIFO byte transfer using atomic
 * head and tail indices with no OS locking.  Storage is caller-supplied;
 * the ring does not allocate.  Partial push and pop are supported, with
 * the return value indicating actual bytes transferred.
 *
 * cdox v1.0.2
 */
#pragma once

#include <stdatomic.h>
#include <stddef.h>
#include <stdint.h>

#include <dtcore/dterr.h>

// --------------------------------------------------------------------------------------
// Simple uint8_t ring buffer with memcpy-based bulk copy in/out.
//
// Concurrency model (lock-free):
//   - Single producer, single consumer (SPSC) only.
//   - Exactly one context may call dtringfifo_push().
//   - Exactly one context may call dtringfifo_pop().
//   - These two contexts may run concurrently without any additional locking.
//   - Initialization / reset must be done in a single-threaded context, or with
//     external synchronization.
//   - If you need multiple producers or multiple consumers, you must add your
//   own
//     higher-level locking around this object.
//
// The implementation does not use any OS-level locks, IRQ masking, or atomics.
// It relies on naturally atomic 32-bit loads/stores for head/tail indices.

typedef struct
{
  uint8_t* buffer;  // storage owned by caller
  int32_t capacity; // number of bytes in buffer (must be >= 2)

  // Indices into [0..capacity). 'head' is the next write position,
  // 'tail' is the next read position.
  //
  // Concurrency:
  //   - Producer: reads tail, writes head.
  //   - Consumer: reads head, writes tail.
  // Atomic indices for SPSC ring
  _Atomic int32_t head; // written by producer, read by consumer
  _Atomic int32_t tail; // written by consumer, read by producer

} dtringfifo_t;

typedef struct
{
  uint8_t* buffer;  // required; caller-owned storage
  int32_t capacity; // required; must be >= 2

} dtringfifo_config_t;

// Initialize to a known empty state. Does NOT allocate or free memory.
extern dterr_t*
dtringfifo_init(dtringfifo_t* self);

// Configure ring over caller-provided storage.
// Does NOT allocate or free memory.
extern dterr_t*
dtringfifo_configure(dtringfifo_t* self, const dtringfifo_config_t* cfg);

// Reset indices and drop contents (buffer pointer/capacity unchanged).
// Safe only if no concurrent push/pop is happening.
extern void
dtringfifo_reset(dtringfifo_t* self);

// Push as many bytes as will fit from src into ring.
// Returns number of bytes actually stored (may be < src_len).
// Caller can treat (src_len - return_value) as "dropped".
extern int32_t
dtringfifo_push(dtringfifo_t* self, const uint8_t* src, int32_t src_len);

// Pop up to dest_len bytes from ring into dest using up to two memcpys.
// Returns number of bytes actually copied (may be 0 if empty).
extern int32_t
dtringfifo_pop(dtringfifo_t* self, uint8_t* dest, int32_t dest_len);

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtringfifo

Lock-free byte ring buffer for Single Producer Single Consumer use

This group of functions provides byte-oriented FIFO buffering for a single producer and a single consumer.
It implements for low-level data transfer between concurrent contexts without external locking.
The implementation uses simple memcpy-based transfers.

## Mini-guide

- Configure the ring over caller-provided storage by requiring explicit initialization and configuration.
- Use exactly one producer and one consumer to preserve the lock-free concurrency model.
- If necessary to reset the ring, do it only when no concurrent access is happening by enforcing external synchronization.

## Example

```c
uint8_t storage[128];
dtringfifo_t fifo;
dtringfifo_config_t cfg = {
    .buffer = storage,
    .capacity = sizeof(storage),
};

dtringfifo_init(&fifo);
dtringfifo_configure(&fifo, &cfg);

dtringfifo_push(&fifo, data, data_len);
count = dtringfifo_pop(&fifo, out, out_len);
```

## Data structures

### dtringfifo_config_t

Describes caller-provided storage used to configure a ring.

Members:

> `uint8_t* buffer` Pointer to caller-owned storage backing the ring.  
> `int32_t capacity` Total number of bytes in the storage buffer.  

### dtringfifo_t

Holds ring state and indices for single-producer single-consumer access.

Members:

> `uint8_t* buffer` Pointer to caller-owned storage backing the ring.  
> `int32_t capacity` Total number of bytes in the storage buffer.  
> `_Atomic int32_t head` Index of the next write position.  
> `_Atomic int32_t tail` Index of the next read position.  

## Functions

### dtringfifo_configure

Associates the ring with caller-provided storage and initializes indices.

Params:

> `dtringfifo_t* self` Ring instance to configure.  
> `const dtringfifo_config_t* cfg` Configuration describing storage and capacity.  

Return: `dterr_t*` Error object on failure, or NULL on success.

### dtringfifo_init

Initializes the ring structure to a known empty state.

Params:

> `dtringfifo_t* self` Ring instance to initialize.  

Return: `dterr_t*` Error object on failure, or NULL on success.

### dtringfifo_pop

Copies available bytes from the ring into the destination buffer.

Params:

> `dtringfifo_t* self` Ring instance to read from.  
> `uint8_t* dest` Destination buffer to receive data.  
> `int32_t dest_len` Maximum number of bytes to copy.  

Return: `int32_t` Number of bytes actually copied.

### dtringfifo_push

Copies bytes from the source buffer into the ring until full.

Params:

> `dtringfifo_t* self` Ring instance to write to.  
> `const uint8_t* src` Source buffer providing data.  
> `int32_t src_len` Number of bytes available in the source buffer.  

Return: `int32_t` Number of bytes actually stored.

### dtringfifo_reset

Resets read and write indices while preserving configured storage.

Params:

> `dtringfifo_t* self` Ring instance to reset.  

Return: `void`  No return value.

<!-- FG_IDC: 945f7e99-44c3-40ae-b3b4-ffa17d79222b | FG_UTC: 2026-01-18T09:10:59Z | FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
