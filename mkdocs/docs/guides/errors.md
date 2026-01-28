# Structured error handling

The `dterr` facility provides a uniform way to report, describe, and propagate failures across
the `dt*` modules.  

Each function that can fail returns a pointer to a `dterr_t` structure.
A `NULL` return indicates success; a non-`NULL` pointer describes an error condition.

The structured error object records:

- a numeric code (enum-like macros `DTERR_*`)
- the source file, line, and function where the error originated
- a formatted message, and
- an optional pointer to an inner (causal) error.

`dterr` offers a consistent pattern for error reporting that carries context without adding dependencies or language extensions.

Please see the [dterr function group](../function_groups/dterr.md) page for more details.

## Intent and rationale
Embedded and systems code often discards useful diagnostic information in favor of terse return codes.
The design goal of `dterr` is to make routine error handling more systematic and informative.
Each error retains enough context to be traced, chained, logged, and examined later.

Key goals:

- **Consistency:** All callable functions report errors the same way.
- **Traceability:** Each node preserves where and why the failure occurred.
- **Isolation:** The mechanism adds no external dependencies and uses standard C allocation rules.
- **Control:** The caller decides whether to print, propagate, or discard an error.

---

## Examples

### Example creation
```c
return dterr_new(DTERR_IO, __FILE__, __func__, __LINE__, NULL, "Read failed on %s", path);
```

### Example chaining
```c
if ((inner_err = child_op())) 
{
    return dterr_new(DTERR_FAIL, __FILE__, __func__, __LINE__, inner_err "Device init failed");
}
```

### Example disposal
```c
// dispose the error and all errors in the chain, if any
dterr_dispose(err);
```

---

### Example logging an error chain

```c
// run your code, which might result in an error or error chain
dterr_t* dterr = my_algorithm();
if (dterr != NULL) 
{
    // log the error chain
    dtlog_dterr(dterr);
    // free memory allocation in the error chain
    dterr_dispose(dterr);

    dterr = null
}
```

### Example iterating an error chain
```c

// --------------------------------------------------------------------------------------
// called sequentially for each error in the chain
static void
_each_error_callback(dterr_t* dterr, void* context)
{
    // examine or do something with the dterr structure
}

// ----------------------------------------------------------------
// iterate over the dterr chain
if (dterr != NULL)
{
    dterr_each(dterr, _each_error_callback, context);
}

```
