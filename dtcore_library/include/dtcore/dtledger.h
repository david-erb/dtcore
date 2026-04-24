/*
 * dtledger -- Per-class allocation accounting with counters and watermarks.
 *
 * Tracks heap allocation and deallocation for named classes via static
 * ledger instances.  Records current balance, cumulative totals, and high/low
 * watermarks for both instance counts and byte usage.  Primarily used in
 * unit tests to verify that all allocations are properly released.
 *
 * cdox v1.0.2
 */
#pragma once

// See markdown documentation at the end of this file.

// A simple system to track instance and byte allocations per class/module via a
// static ledger.
//
// The mechanism is manual: the user must invoke increment/decrement macros at
// appropriate points in their allocation and free paths.

#include <sys/types.h>

#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

struct dtledger_t;
typedef struct dtledger_t dtledger_t;

struct dtledger_t
{
  const char* class_name;

  int32_t count_total;

  int32_t count_balance;

  int32_t count_high_water_mark;

  int32_t count_low_water_mark;

  int32_t bytes_total;

  int32_t bytes_balance;

  int32_t bytes_high_water_mark;

  int32_t bytes_low_water_mark;
};

extern void
dtledger_to_string(dtledger_t* ledger, char* buffer, int32_t buffer_size);

// Note: Use ::DTLEDGER_REGISTER in exactly one `.c` file to define the storage;
//       all other translation units should include this declaration.
#define DTLEDGER_DECLARE(CLASS_NAME) extern dtledger_t* CLASS_NAME##_ledger;

// Warning: Call this macro **once** per class in a single `.c` file to avoid
// duplicate definitions.
#define DTLEDGER_REGISTER(CLASS_NAME)                                          \
  static dtledger_t _##CLASS_NAME##_ledger = { .class_name = #CLASS_NAME };    \
  dtledger_t* CLASS_NAME##_ledger = &_##CLASS_NAME##_ledger;

//

// Note: Updates totals, balance, and high-water marks. Call on the success path
// only.
#define DTLEDGER_INCREMENT(CLASS_NAME, BYTES)                                  \
  CLASS_NAME##_ledger->count_balance++;                                        \
  CLASS_NAME##_ledger->count_total++;                                          \
  if (CLASS_NAME##_ledger->count_balance >                                     \
      CLASS_NAME##_ledger->count_high_water_mark) {                            \
    CLASS_NAME##_ledger->count_high_water_mark =                               \
      CLASS_NAME##_ledger->count_balance;                                      \
  }                                                                            \
  CLASS_NAME##_ledger->bytes_balance += BYTES;                                 \
  CLASS_NAME##_ledger->bytes_total += BYTES;                                   \
  if (CLASS_NAME##_ledger->bytes_balance >                                     \
      CLASS_NAME##_ledger->bytes_high_water_mark) {                            \
    CLASS_NAME##_ledger->bytes_high_water_mark =                               \
      CLASS_NAME##_ledger->bytes_balance;                                      \
  }

// Note: Updates balance and low-water marks. Safe to pass 0 when only instance
// counts matter. Warning: Keep increments/decrements balanced to avoid negative
// balances and misleading watermarks.
#define DTLEDGER_DECREMENT(CLASS_NAME, BYTES)                                  \
  CLASS_NAME##_ledger->count_balance--;                                        \
  if (CLASS_NAME##_ledger->count_balance <                                     \
      CLASS_NAME##_ledger->count_low_water_mark) {                             \
    CLASS_NAME##_ledger->count_low_water_mark =                                \
      CLASS_NAME##_ledger->count_balance;                                      \
  }                                                                            \
  CLASS_NAME##_ledger->bytes_balance -= BYTES;                                 \
  if (CLASS_NAME##_ledger->bytes_balance <                                     \
      CLASS_NAME##_ledger->bytes_low_water_mark) {                             \
    CLASS_NAME##_ledger->bytes_low_water_mark =                                \
      CLASS_NAME##_ledger->bytes_balance;                                      \
  }

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtledger

Manual allocation accounting with per-class counters and watermarks

This group of functions provides manual allocation tracking for per-class ledgers. 
It provides simple visibility into instance and byte usage.
Mainly this is used in unit tests to ensure memory is not leaked.
The implementation is done with low overhead and no dynamic allocation.

At the current time, there are ledgers for:

- `dtbuffer` for heap-backed buffer descriptors
- `dtheaper` for raw heap allocations
- `dtstr` for dynamic strings
- `dterr` for error objects with heap-allocated messages

## Mini-guide

- Define the static storage exactly once per tracked class by using `DTLEDGER_REGISTER`  
- Reference the ledger from other translation units by using `DTLEDGER_DECLARE`.  
- Increment counters only on successful allocation paths by using `DTLEDGER_INCREMENT`  
- Decrement counters on free paths by using `DTLEDGER_DECREMENT`.  
- Access the ledger structure members directly for custom reporting.
- Format a human-readable summary by calling `dtledger_to_string`.  

## Example

```c
#include <inttypes.h>
#include <dtcore/dtledger.h>

DTLEDGER_DECLARE(Foo)

void example(void)
{
    DTLEDGER_REGISTER(Foo);

    DTLEDGER_INCREMENT(Foo, 128);
    DTLEDGER_DECREMENT(Foo, 128);

    char buf[256];
    dtledger_to_string(Foo_ledger, buf, (int32_t)sizeof(buf));
}
```

## Data structures

### dtledger_t

This type holds counters and watermarks for a named ledger.

Members:

> `const char* class_name` Name associated with the ledger.  
> `int32_t bytes_balance` Current byte balance.  
> `int32_t bytes_high_water_mark` Highest observed byte balance.  
> `int32_t bytes_low_water_mark` Lowest observed byte balance.  
> `int32_t bytes_total` Cumulative bytes added to the ledger.  
> `int32_t count_balance` Current instance balance.  
> `int32_t count_high_water_mark` Highest observed instance balance.  
> `int32_t count_low_water_mark` Lowest observed instance balance.  
> `int32_t count_total` Cumulative instances added to the ledger.  

## Macros

### DTLEDGER_DECLARE

`DTLEDGER_DECLARE(CLASS_NAME)`

Declares an external `dtledger_t*` for a class-specific ledger.

### DTLEDGER_DECREMENT

`DTLEDGER_DECREMENT(CLASS_NAME, BYTES)`

Decrements the ledger balances and updates low-water marks.

### DTLEDGER_INCREMENT

`DTLEDGER_INCREMENT(CLASS_NAME, BYTES)`

Increments the ledger totals and balances and updates high-water marks.

### DTLEDGER_REGISTER

`DTLEDGER_REGISTER(CLASS_NAME)`

Defines storage for a class-specific ledger and exposes a `dtledger_t*` pointer to it.

## Functions

### dtledger_to_string

Formats the current ledger counters into a caller-provided buffer.

Params:

> `dtledger_t* ledger` Ledger to format.  

> `char* buffer` Destination buffer to receive the formatted text.  

> `int32_t buffer_size` Size of `buffer` in bytes.  

Return: `void`  No return value.  

<!-- FG_IDC: d95e073b-c773-46e7-a1e7-fa3a57b09917 | FG_UTC: 2026-01-17T10:11:22Z FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
