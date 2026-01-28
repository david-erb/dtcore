#pragma once
// See markdown documentation at the end of this file.

// Split a source buffer into fixed-size chunks and reassemble them.

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dtbuffer.h>
#include <dtcore/dterr.h>
#include <dtcore/dtlog.h>

#include <dtcore/dtchunker.h>

//
// Used by dtchunker_import() to validate the stream.
#define DTCHUNKER_MAGIC 0xDEADBEEF

//
// Packed/unpacked via `dtpackx` helpers (two consecutive `int32_t` values).
typedef struct dtchunker_header_t
{
  int32_t magic;                ///< Magic value identifying this wire format.
  int32_t source_buffer_length; ///< Total length of the original source buffer
                                ///< in bytes.
} dtchunker_header_t;

//
// Export and import modes are mutually exclusive.
typedef enum dtchunker_mode_t
{
  DTCHUNKER_MODE_UNKNOWN, ///< Not yet chosen; first export/import call sets it.
  DTCHUNKER_MODE_IMPORT,  ///< Reassembling chunks; only dtchunker_import()
                          ///< allowed.
  DTCHUNKER_MODE_EXPORT ///< Producing chunks; only dtchunker_export() allowed.
} dtchunker_mode_t;

//
// **Ownership & lifetime**
// - During export, dtchunker_export() returns a wrapped buffer that aliases
//   internal storage (valid until next export).
// - During import, once all bytes are received, dtchunker_import() returns
//   the fully reconstructed dtbuffer_t and transfers ownership.
//
// **Concurrency**
// - Not thread-safe or interrupt-safe; external synchronization required.
typedef struct dtchunker_t
{
  dtbuffer_t*
    working_buffer; ///< Internal heap buffer for payload slices and reassembly.
  dtbuffer_t
    wrapped_buffer; ///< Points into `working_buffer` for zero-copy returns.
  int32_t
    working_index; ///< Current byte index within the source/target buffer.
  dtchunker_mode_t mode;  ///< Current operating mode.
  int32_t max_chunk_size; ///< Max payload bytes per chunk when exporting (≥ 8).
  int32_t header_length;  ///< Serialized header length in bytes.
} dtchunker_t;

// --------------------------------------------------------------------------------------------

//
// Sets mode to DTCHUNKER_MODE_UNKNOWN and prepares internal header length.
//
//
// Note: After success, call dtchunker_export() **or** dtchunker_import() to
// choose mode.
extern dterr_t*
dtchunker_init(dtchunker_t* self, int32_t max_chunk_size);

//
// Keeps allocated internal buffers for reuse; safe between sessions.
//
extern dterr_t*
dtchunker_reset(dtchunker_t* self);

//
// - On first call, returns a header-only chunk describing the stream.
// - Subsequent calls return payload slices up to `max_chunk_size`.
// - When done, sets `*chunk_buffer` to `NULL` and resets the chunker.
//
//
// Warning: The returned `*chunk_buffer` aliases internal storage and remains
// valid
//          only until the next call to dtchunker_export().
extern dterr_t*
dtchunker_export(dtchunker_t* self,
                 dtbuffer_t* source_buffer,
                 dtbuffer_t** chunk_buffer);

//
// Accepts either a header-only first chunk or a header + payload in one.
// Returns `*final_buffer == NULL` until all bytes are received; then yields the
// completed dtbuffer_t and transfers ownership.
//
//
// Warning: When `*final_buffer != NULL`, the chunker releases its pointer and
//          the caller becomes the owner of `*final_buffer`.
extern dterr_t*
dtchunker_import(dtchunker_t* self,
                 dtbuffer_t* chunk_buffer,
                 dtbuffer_t** final_buffer);

//
// Safe to call on zero-initialized or already-disposed instances.
// After disposal, re-initialize with dtchunker_init().
//
extern void
dtchunker_dispose(dtchunker_t* self);

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtchunker

Chunked buffer export and import with reassembly.

This group of functions provides chunking and reassembly for byte buffers.
It is intended for streaming fixed-size payloads across transports with size limits.
It preserves ordering and total length across sessions.
The implementation does simple state tracking and zero-copy slices where possible.

## Mini-guide

- Initialize once per session by calling dtchunker_init with a valid maximum chunk size.
- Choose a mode implicitly by calling either the export or import path first.
- Emit a header chunk first during export, then consume payload chunks sequentially.
- Provide chunks in order during import until completion is reported.
- Reset or dispose between independent sessions to clear internal state.

## Example

```c
dtchunker_t chunker;
dtbuffer_t* chunk = NULL;
dtbuffer_t* final = NULL;

dtchunker_init(&chunker, 256);

while (dtchunker_export(&chunker, source, &chunk) == NULL && chunk != NULL)
{
    send_chunk(chunk);
}

dtchunker_reset(&chunker);

while (receive_chunk(&chunk))
{
    dtchunker_import(&chunker, chunk, &final);
    if (final != NULL)
    {
        use_buffer(final);
        dtbuffer_dispose(final);
        break;
    }
}

dtchunker_dispose(&chunker);
```

## Data structures

### dtchunker_header_t

Describes the serialized header for a chunked stream.

Members:

> `int32_t magic` Magic value identifying the wire format.  
>
> `int32_t source_buffer_length` Total length of the original source buffer in bytes.  

### dtchunker_mode_t

Defines the current operating mode of a chunker instance.

Members:

> `DTCHUNKER_MODE_EXPORT` Producing chunks from a source buffer.  
>
> `DTCHUNKER_MODE_IMPORT` Reassembling chunks into a destination buffer.  
>
> `DTCHUNKER_MODE_UNKNOWN` Mode not yet selected.  

### dtchunker_t

Holds internal state for chunk export or import sessions.

Members:

> `dtbuffer_t* working_buffer` Internal heap buffer for payload slices and reassembly.  
>
> `dtbuffer_t wrapped_buffer` Wrapper pointing into internal storage for zero-copy returns.  
>
> `int32_t working_index` Current byte index within the source or target buffer.  
>
> `dtchunker_mode_t mode` Current operating mode.  
>
> `int32_t max_chunk_size` Maximum payload bytes per exported chunk.  
>
> `int32_t header_length` Serialized header length in bytes.  

## Macros

### DTCHUNKER_MAGIC

Identifies the expected wire format for chunked streams.

## Functions

### dtchunker_dispose

Releases internal resources and clears the instance state.

Params:

> `dtchunker_t* self` Chunker instance to dispose.  

Return: `void` No return value.

### dtchunker_export

Produces the next header or payload chunk from a source buffer.

Params:

> `dtchunker_t* self` Chunker instance managing the export session.  
>
> `dtbuffer_t* source_buffer` Source buffer to split into chunks.  
>
> `dtbuffer_t** chunk_buffer` Output pointer receiving the current chunk or NULL when complete.  

Return: `dterr_t*` Error status or NULL on success.

### dtchunker_import

Consumes a header or payload chunk and assembles the final buffer.

Params:

> `dtchunker_t* self` Chunker instance managing the import session.  
>
> `dtbuffer_t* chunk_buffer` Incoming chunk buffer to process.  
>
> `dtbuffer_t** final_buffer` Output pointer receiving the completed buffer when finished.  

Return: `dterr_t*` Error status or NULL on success.

### dtchunker_init

Initializes a chunker instance with a maximum chunk size.

Params:

> `dtchunker_t* self` Chunker instance to initialize.  
>
> `int32_t max_chunk_size` Maximum payload size per exported chunk in bytes.  

Return: `dterr_t*` Error status or NULL on success.

### dtchunker_reset

Resets internal state while keeping allocated buffers for reuse.

Params:

> `dtchunker_t* self` Chunker instance to reset.  

Return: `dterr_t*` Error status or NULL on success.

<!-- FG_IDC: 9bd789af-ba16-434b-bbc2-a17dd44d36b5 | FG_UTC: 2026-01-17T10:26:00Z FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
