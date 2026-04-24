/*
 * dterr -- Structured error capture and chaining with source location.
 *
 * Each error is a heap-allocated node carrying a code, source file, function,
 * line number, formatted message, and an optional inner cause.  Macros
 * standardize the goto-cleanup error-propagation pattern used throughout
 * the library.  Errors can be iterated in causal order and disposed as
 * a chain with a single call.
 *
 * cdox v1.0.2
 */
#pragma once

// See markdown documentation at the end of this file.

// Error capture and propagation.

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include <dtcore/dtledger.h>
//
// Note: Ledger accounting is done in (@ref dtledger) for all allocations and
// deallocations made by these functions.

// Declare a ledger channel named "dterr" for allocation accounting.
DTLEDGER_DECLARE(dterr);

#define DTERR_OK 0              ///< Operation succeeded; typically no ::dterr_t* is returned.
#define DTERR_FAIL 1            ///< Generic failure when no better code applies.
#define DTERR_ASSERTION 2       ///< Internal assertion failed (usually indicates a programming error).
#define DTERR_BADARG 3          ///< Invalid argument value or shape.
#define DTERR_NOMEM 4           ///< Memory allocation failed (out of memory).
#define DTERR_IO 5              ///< I/O error (read/write/open).
#define DTERR_INTERRUPT 6       ///< Interrupted by signal or external event.
#define DTERR_TIMEOUT 7         ///< Operation timed out.
#define DTERR_NOTFOUND 8        ///< Requested item was not found.
#define DTERR_EXISTS 9          ///< Attempt to create an item that already exists.
#define DTERR_NOTIMPL 10        ///< Feature or code path is not implemented.
#define DTERR_UNREACHABLE 11    ///< Code path should be unreachable if invariants hold.
#define DTERR_UNEXPECTED 12     ///< Unexpected state or input encountered.
#define DTERR_OVERFLOW 13       ///< Numeric or capacity overflow.
#define DTERR_UNDERFLOW 14      ///< Numeric or capacity underflow.
#define DTERR_RANGE 15          ///< Argument out of valid range.
#define DTERR_BADCONFIG 16      ///< Invalid or inconsistent configuration.
#define DTERR_ARGUMENT_NULL 17  ///< Required argument was `NULL`.
#define DTERR_BUSY 18           ///< Resource is busy or locked.
#define DTERR_STATE 19          ///< Invalid state for requested operation.
#define DTERR_INFRASTRUCTURE 20 ///< External service or infrastructure failure.
#define DTERR_DELIBERATE 21     ///< Intentional failure for tests/tools to short-circuit control flow.
#define DTERR_INTERNAL 22       ///< Internal error (indicates a bug in the library).
#define DTERR_BOUNDS 23         ///< Index or pointer out of bounds.
#define DTERR_CORRUPT 24        ///< Data corruption detected.
#define DTERR_NOTREADY 25       ///< Not ready for requested operation.
#define DTERR_BADMAGIC 26       ///< Invalid magic number or signature.
#define DTERR_OS 27             ///< Operating system error.

// Warning: Requires a `dterr_t* dterr` variable in scope and a `cleanup:` label.
#define DTERR_C(call)                                                                                                          \
    if ((dterr = (call)) != NULL)                                                                                              \
    {                                                                                                                          \
        dterr = dterr_new(dterr->error_code, DTERR_LOC, dterr, "traceback");                                                   \
        goto cleanup;                                                                                                          \
    }

// Appends `call`'s error to `dterr`, or assigns it if `dterr` is NULL
#define DTERR_APPEND(call)                                                                                                     \
    do                                                                                                                         \
    {                                                                                                                          \
        dterr_t* _call_err;                                                                                                    \
        if ((_call_err = (call)) != NULL)                                                                                      \
        {                                                                                                                      \
            if (dterr == NULL)                                                                                                 \
            {                                                                                                                  \
                dterr = _call_err;                                                                                             \
            }                                                                                                                  \
            else                                                                                                               \
            {                                                                                                                  \
                dterr_append(dterr, _call_err);                                                                                \
            }                                                                                                                  \
        }                                                                                                                      \
    } while (0);

// Warning: Jumps to `cleanup:` on failure and assigns ::DTERR_ARGUMENT_NULL to
// `dterr`.
#define DTERR_ASSERT_NOT_NULL(ARG)                                                                                             \
    if ((ARG) == NULL)                                                                                                         \
    {                                                                                                                          \
        dterr = dterr_new(DTERR_ARGUMENT_NULL, DTERR_LOC, NULL, "%s is NULL", #ARG);                                           \
        goto cleanup;                                                                                                          \
    }

// ----------------------------------------------------------------------------
// errno-based format string tokens
#define DTERR_ERRNO_FORMAT "error %d (%s)"

// errno-based format string arguments
#define DTERR_ERRNO_ARGS() (errno, strerror(errno) ? strerror(errno) : "unknown error")

// errno-based return values
#define DTERR_ERRNO_C(call)                                                                                                    \
    do                                                                                                                         \
    {                                                                                                                          \
        int err;                                                                                                               \
        if ((err = (call)) != 0)                                                                                               \
        {                                                                                                                      \
            dterr = dterr_new(DTERR_FAIL, DTERR_LOC, NULL, DTERR_ERRNO_FORMAT, DTERR_ERRNO_ARGS());                            \
            goto cleanup;                                                                                                      \
        }                                                                                                                      \
    } while (0);

// ----------------------------------------------------------------------------
// positive strerror-based format string tokens
#define DTERR_POSERROR_FORMAT "error %d (%s)"

// positive strerror-based format string arguments
#define DTERR_POSERROR_ARGS(ERR) (ERR), ((ERR) < 0 ? strerror(ERR) : "unknown error")

// positive strerror-based return values
#define DTERR_POSERROR_C(call)                                                                                                 \
    do                                                                                                                         \
    {                                                                                                                          \
        int err;                                                                                                               \
        if ((err = (call)) != 0)                                                                                               \
        {                                                                                                                      \
            dterr = dterr_new(DTERR_FAIL, DTERR_LOC, NULL, DTERR_POSERROR_FORMAT, DTERR_POSERROR_ARGS(err));                   \
            goto cleanup;                                                                                                      \
        }                                                                                                                      \
    } while (0);

// ----------------------------------------------------------------------------
// negative strerror-based format string tokens
#define DTERR_NEGERROR_FORMAT "error %d (%s)"

// negative strerror-based format string arguments
#define DTERR_NEGERROR_ARGS(ERR) (ERR), ((ERR) < 0 ? strerror(-(ERR)) : "unknown error")

// negative strerror-based return values
#define DTERR_NEGERROR_C(call)                                                                                                 \
    do                                                                                                                         \
    {                                                                                                                          \
        int err;                                                                                                               \
        if ((err = (call)) != 0)                                                                                               \
        {                                                                                                                      \
            dterr = dterr_new(DTERR_FAIL, DTERR_LOC, NULL, DTERR_NEGERROR_FORMAT, DTERR_NEGERROR_ARGS(err));                   \
            goto cleanup;                                                                                                      \
        }                                                                                                                      \
    } while (0);
// ----------------------------------------------------------------------------

struct dterr_t;
typedef struct dterr_t dterr_t;

struct dterr_t
{
    int32_t error_code;                    ///< One of `DTERR_` codes indicating the primary failure.
    int32_t line_number;                   ///< Source line where the error was created (see ::DTERR_LOC).
    const char* source_file;               ///< Source file of origin; static storage duration.
    const char* source_function;           ///< Function name of origin; static storage duration.
    dterr_t* inner_err;                    ///< Optional inner/caused-by error; may be `NULL`.
    char* message;                         ///< Heap string owned by this node; `NULL` if no message.
    void (*dispose)(struct dterr_t* self); ///< Virtual destructor for the node.
};

// Note: The returned node owns its `message` and (if provided) `inner_err`.
// Call ::dterr_dispose() to free the entire chain.
dterr_t*
dterr_new(int32_t error_code,
  int32_t line_number,
  const char* source_file,
  const char* source_function,
  dterr_t* inner_err,
  const char* format,
  ...);

//
// Expands to `__LINE__, __FILE__, __func__`.
#define DTERR_LOC __LINE__, __FILE__, __func__

typedef void (*dterr_each_callback_t)(dterr_t* err, void* context);

// Note: Iteration order matches causal order: inner cause first, then outer
// wrappers.
void
dterr_each(dterr_t* self, dterr_each_callback_t callback, void* context);

// Warning: Appending transfers ownership of `that` into `self`'s chain; do not
// dispose `that` separately.
dterr_t*
dterr_append(dterr_t* self, dterr_t* that);

// Note: Safe to call on partially constructed nodes; frees the owned `message`
// and chained errors. Note: Dispose with ::dterr_dispose() to keep the ledger
// accurate.
void
dterr_dispose(dterr_t* self);

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dterr

Error capture, chaining, and structured propagation.

This group of functions provides error capture and propagation for C modules.
It is designed to construct explicit error chains with source location and messages.
The implementation does explicit ownership and disposal over implicit global state.

## Mini-guide

- Construct error nodes explicitly by calling `dterr_new`, which formats a message into heap memory.
- Propagate failures through wrappers by appending or nesting errors.
- Use the provided macros to short-circuit control flow which jump to a `cleanup:`.
- Iterate error chains with `dterr_each`.
- Dispose entire error chains with `dterr_dispose`.

## Example

```c
dterr_t* dterr = NULL;

DTERR_ASSERT_NOT_NULL(some_ptr);

dterr = dterr_new(DTERR_FAIL, DTERR_LOC, NULL, "operation failed");

cleanup:
if (dterr != NULL)
{
    dterr_each(dterr, some_callback, NULL);
    dterr_dispose(dterr);
}
```

## Data structures

### dterr_t

Represents a single error node with optional causal linkage.

Members:

> `int32_t error_code` Primary error classification code.  
> `int32_t line_number` Source line where the error was created.  
> `const char* source_file` Source file of origin with static storage duration.  
> `const char* source_function` Function name of origin with static storage duration.  
> `dterr_t* inner_err` Optional inner error representing the causal predecessor.  
> `char* message` Heap-allocated message owned by this node.  
> `void (*dispose)(struct dterr_t* self)` Virtual destructor for the node.  

## Macros

### DTERR_ASSERT_NOT_NULL

`DTERR_ASSERT_NOT_NULL(ARG)`

Checks a required argument and creates an error node before jumping to cleanup.

### DTERR_C

`DTERR_C(call)`

Wraps a call and converts a returned error into a chained error node before cleanup.


### DTERR_ERRNO_C

`DTERR_ERRNO_C(call)`

Wraps an errno-style call and creates an error node on nonzero return.

### DTERR_NEGERROR_C

`DTERR_NEGERROR_C(call)`

Wraps a call returning negative error codes and creates an error node on failure.


### DTERR_POSERROR_C

Wraps a call returning positive error codes and creates an error node on failure.


## Functions

### dterr_append

Appends an error chain to another by transferring ownership.

Params:

> `dterr_t* self` Destination error node that receives the appended chain.  
> `dterr_t* that` Source error node whose chain is transferred.  

Return: `dterr_t *`  New head of the error chain.

### dterr_dispose

Disposes an error chain and releases all owned resources.

Params:

> `dterr_t* self` Error node to dispose.  

Return: `void`  No return value.

### dterr_each

Invokes a callback for each error node in causal order.

Params:

> `dterr_t* self` Error chain to iterate.  
> `dterr_each_callback_t callback` Callback invoked for each node.  
> `void* context` Caller-provided context passed to the callback.  

Return: `void`  No return value.

### dterr_new

Creates a new error node with formatted message text.

Params:

> `int32_t error_code` Primary error classification code.  
> `int32_t line_number` Source line where the error was created.  
> `const char* source_file` Source file of origin.  
> `const char* source_function` Function name of origin.  
> `dterr_t* inner_err` Optional inner error representing the cause.  
> `const char* format` Format string for the message.  
> `...` Variadic arguments for the format string.  

Return: `dterr_t*` Newly created error node or `NULL` on construction failure.

<!-- FG_IDC: 4d9cd5b7-a14d-4eee-87b2-28573d6eaa5c | FG_UTC: 2026-01-17T10:01:33Z | FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
