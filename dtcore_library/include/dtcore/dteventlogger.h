/*
 * dteventlogger -- Fixed-size ring buffer for timestamped event logging.
 *
 * Accumulates a bounded number of fixed-width event records, overwriting
 * the oldest entries when full.  Supports snapshot cloning for safe
 * inspection under concurrent writes, and formats item1 records to the
 * log facility or into an allocated string.
 *
 * cdox v1.0.2
 */
#pragma once
// See markdown documentation at the end of this file.

#include <dtcore/dterr.h>

#include <dtcore/dtbuffer.h>

typedef struct dteventlogger_t
{
  int32_t item_count, item_length;
  uint32_t buffer_size; // size of the buffer in bytes
  dtbuffer_t* buffer;   // the buffer that holds the events
  volatile uint32_t write_index;
} dteventlogger_t;

typedef struct dteventlogger_item0_t
{
  uint64_t timestamp; // timestamp of the event
} dteventlogger_item0_t;

typedef struct dteventlogger_item1_t
{
  uint64_t timestamp; // timestamp of the event
  int32_t value1;     // the first value of the event
  int32_t value2;     // the second value of the event
} dteventlogger_item1_t;

// --------------------------------------------------------------------------------------------
extern dterr_t*
dteventlogger_init(dteventlogger_t* self,
                   int32_t item_count,
                   int32_t item_length);

extern dterr_t*
dteventlogger_clone(dteventlogger_t* self, dteventlogger_t* clone);
extern dterr_t*
dteventlogger_append(dteventlogger_t* self, const void* item);
extern dterr_t*
dteventlogger_get_item(dteventlogger_t* self, int32_t index, void** item);
extern dterr_t*
dteventlogger_log_item1( //
  const char* tag,
  dteventlogger_t* eventlogger,
  const char* logger_label,
  const char* value1_label,
  const char* value2_label);
dterr_t*
dteventlogger_printf_item1( //
  dteventlogger_t* eventlogger,
  const char* logger_label,
  const char* value1_label,
  const char* value2_label,
  char** out_string);
extern void
dteventlogger_dispose(dteventlogger_t* self);

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dteventlogger

Fixed-size event logging with timestamped items.

This group of functions provides fixed-size event logging for in-memory event streams.
It is intended for lightweight diagnostics and inspection in embedded or low-level runtime contexts.
It is designed to .
The implementation accumulates recent events without dynamic growth 
resulting in deterministic memory usage.

## Mini-guide

- Initialize the logger with a fixed item size and count.
- Append events sequentially, overflow will overwrite the oldest entries.
- Clone the logger state for inspection by isolating concurrent writes.

## Example

```c
dteventlogger_t logger;
dteventlogger_item1_t item;

dteventlogger_init(&logger, 16, sizeof(item));

item.timestamp = 1000;
item.value1 = 42;
item.value2 = 7;
dteventlogger_append(&logger, &item);

dteventlogger_log_item1("APP", &logger, "events", "v1", "v2");

dteventlogger_dispose(&logger);
```

## Data structures

### dteventlogger_item0_t

Represents a minimal event item containing only a timestamp.

Members:

> `uint64_t timestamp` Timestamp of the event.  

### dteventlogger_item1_t

Represents an event item with a timestamp and two associated values.

Members:

> `uint64_t timestamp` Timestamp of the event.  
> `int32_t value1` First value associated with the event.  
> `int32_t value2` Second value associated with the event.  

### dteventlogger_t

Tracks the state of a fixed-size event logger and its backing buffer.

Members:

> `int32_t item_count` Maximum number of items retained.  
> `int32_t item_length` Size in bytes of each item.  
> `uint32_t buffer_size` Size of the backing buffer in bytes.  
> `dtbuffer_t* buffer` Buffer that holds the raw event data.  
> `volatile uint32_t write_index` Monotonic index of the next write position.  

## Functions

### dteventlogger_append

Appends a single item to the logger buffer.

Params:

> `dteventlogger_t* self` Event logger instance.  
> `const void* item` Pointer to the item data to append.  

Return: `dterr_t*` Error object on failure, otherwise NULL.

### dteventlogger_clone

Creates a snapshot copy of the logger state and buffer.

Params:

> `dteventlogger_t* self` Source event logger instance.  
> `dteventlogger_t* clone` Destination logger to initialize and populate.  

Return: `dterr_t*` Error object on failure, otherwise NULL.

### dteventlogger_dispose

Releases resources associated with the logger.

Params:

> `dteventlogger_t* self` Event logger instance.  

Return: `void`  No return value.

### dteventlogger_get_item

Retrieves an item by index from the logger buffer.

Params:

> `dteventlogger_t* self` Event logger instance.  
> `int32_t index` Zero-based index relative to the oldest retained item.  
> `void** item` Output pointer to the item data or NULL.  

Return: `dterr_t*` Error object on failure, otherwise NULL.

### dteventlogger_init

Initializes an event logger with fixed item sizing.

Params:

> `dteventlogger_t* self` Event logger instance to initialize.  
> `int32_t item_count` Number of items to retain.  
> `int32_t item_length` Size in bytes of each item.  

Return: `dterr_t*` Error object on failure, otherwise NULL.

### dteventlogger_log_item1

Logs formatted item1 events using the logging facility.

Params:

> `const char* tag` Logging tag to associate with the output.  
> `dteventlogger_t* eventlogger` Event logger instance.  
> `const char* logger_label` Label for the logger output.  
> `const char* value1_label` Label for the first value column.  
> `const char* value2_label` Label for the second value column.  

Return: `dterr_t*` Error object on failure, otherwise NULL.

### dteventlogger_printf_item1

Formats item1 events into a newly allocated string.

Params:

> `dteventlogger_t* eventlogger` Event logger instance.  
> `const char* logger_label` Label for the logger output.  
> `const char* value1_label` Label for the first value column.  
> `const char* value2_label` Label for the second value column.  
> `char** out_string` Output pointer receiving the allocated string.  

Return: `dterr_t*` Error object on failure, otherwise NULL.

<!-- FG_IDC: fc78086d-d86d-4819-b144-fbf4c5c7f063 | FG_UTC: 2026-01-17T10:28:50Z FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
