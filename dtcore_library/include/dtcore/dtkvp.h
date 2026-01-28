#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include <dtcore/dtbuffer.h>
#include <dtcore/dterr.h>
#include <dtcore/dtpackable.h>

#include <dtcore/dtpackx.h>

// Key-value pair structure, each item is a pair of dtstr_dup'd strings.
typedef struct dtkvp_t
{
  const char* key;
  const char* value;
} dtkvp_t;

// simple list of key-value pairs, grows as needed
typedef struct dtkvp_list_t
{
  dtkvp_t* items;
  int32_t count;
  dtbuffer_t* storage;
} dtkvp_list_t;

// initialize the kvp list, allocating internal storage
extern dterr_t*
dtkvp_list_init(dtkvp_list_t* self);

// set a key-value pair, overwrites any existing value, if value is NULL,
// removes the key
extern dterr_t*
dtkvp_list_set(dtkvp_list_t* self, const char* key, const char* value);

// get a value by key, returns value as NULL if key is not present
extern dterr_t*
dtkvp_list_get(dtkvp_list_t* self, const char* key, const char** value);

// get length needed to pack the kvp list
extern dterr_t*
dtkvp_list_packx_length(dtkvp_list_t* self DTPACKABLE_PACKX_LENGTH_ARGS);

// pack the kvp list into the output buffer, error if buffer too small
extern dterr_t*
dtkvp_list_packx(dtkvp_list_t* self DTPACKABLE_PACKX_ARGS);

// unpack the kvp list from the input buffer
extern dterr_t*
dtkvp_list_unpackx(dtkvp_list_t* self DTPACKABLE_UNPACKX_ARGS);

// write kvp as key=value\n strings extending the provided buffer
extern dterr_t*
dtkvp_list_compose_plain_text(dtkvp_list_t* self,
                              char** s,
                              const char* separator);

// dispose the kvp list and all its contents, tolerates a null pointer or
// partially initialized list
extern void
dtkvp_list_dispose(dtkvp_list_t* self);

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtkvp

Resizable key-value list with pack and text output.

This group of functions provides a key-value pair list, similar to a dictionary or map.
It is intended for callers that need mutable string key-value storage with optional serialization.
It is designed to support set, lookup, and encoding and dynamic growth.

## Mini-guide

- Initialize a list before use by calling the init function to allocate internal storage.  
- Set or update an entry by calling the set function.  
- Remove an entry by calling the set function with a null value.  
- Read a value by calling the get function, which returns a stored pointer or a null pointer when the key is absent.  
- Serialize or deserialize list contents by calling the pack and unpack functions.  
- Release all resources by calling the dispose function.  

## Example

```c
#include <dtcore/dtkvp.h>
#include <dtcore/dtstr.h>

static void demo(void)
{
    dtkvp_list_t list;
    const char* v = NULL;
    char* text = NULL;

    dtkvp_list_init(&list);

    dtkvp_list_set(&list, "mode", "demo");
    dtkvp_list_set(&list, "count", "1");

    dtkvp_list_get(&list, "mode", &v);

    dtkvp_list_compose_plain_text(&list, &text, "\n");

    dtstr_dispose(text);
    dtkvp_list_dispose(&list);
}
```

## Data structures

### dtkvp_list_t

This type tracks a resizable list of key-value pairs with internal string storage.

Members:

> `dtkvp_t* items` Pointer to the current item array.  
> `int32_t count` Number of allocated slots in the item array.  
> `dtbuffer_t* storage` Buffer that owns the item array allocation.  

### dtkvp_t

This type tracks a single key and value string pair.

Members:

> `const char* key` Pointer to the stored key string.  
> `const char* value` Pointer to the stored value string.  

## Functions

### dtkvp_list_compose_plain_text

This function appends entries to a caller-managed string buffer as key and value text.

Params:

> `dtkvp_list_t* self` List instance to read from.  
> `char** s` Pointer to a mutable string pointer that is extended by the function.  
> `const char* separator` String inserted between composed entries.  

Return: `dterr_t*` Error object on failure, or null on success.

### dtkvp_list_dispose

This function releases list storage and any duplicated strings.

Params:

> `dtkvp_list_t* self` List instance to dispose, or null.  

Return: `void`  No return value.

### dtkvp_list_get

This function looks up a value pointer for a key in the list.

Params:

> `dtkvp_list_t* self` List instance to query.  
> `const char* key` Key string to match.  
> `const char** value` Output pointer that receives the stored value pointer or null.  

Return: `dterr_t*` Error object on failure, or null on success.

### dtkvp_list_init

This function initializes a list instance and allocates its internal storage.

Params:

> `dtkvp_list_t* self` List instance to initialize.  

Return: `dterr_t*` Error object on failure, or null on success.

### dtkvp_list_packx

This function packs list contents into a caller-provided buffer in packx format.

Params:

> `dtkvp_list_t* self` List instance to pack.  
> `DTPACKABLE_PACKX_ARGS` Pack arguments as required by the packx interface.  

Return: `dterr_t*` Error object on failure, or null on success.

### dtkvp_list_packx_length

This function computes the length required to pack list contents in packx format.

Params:

> `dtkvp_list_t* self` List instance to measure.  
> `DTPACKABLE_PACKX_LENGTH_ARGS` Length arguments as required by the packx length interface.  

Return: `dterr_t*` Error object on failure, or null on success.

### dtkvp_list_set

This function inserts, updates, or removes a key-value entry in the list.

Params:

> `dtkvp_list_t* self` List instance to modify.  
> `const char* key` Key string to match or insert.  
> `const char* value` Value string to store, or null to remove the key.  

Return: `dterr_t*` Error object on failure, or null on success.

### dtkvp_list_unpackx

This function unpacks list contents from a caller-provided buffer in packx format.

Params:

> `dtkvp_list_t* self` List instance to receive entries.  
> `DTPACKABLE_UNPACKX_ARGS` Unpack arguments as required by the unpackx interface.  

Return: `dterr_t*` Error object on failure, or null on success.
<!-- FG_IDC: c2e7da30-e52f-4a30-992d-d28460885eb4 | FG_UTC: 2026-01-17T12:52:30Z FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
