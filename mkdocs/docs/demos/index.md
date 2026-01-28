# Demos

The demo applications are small, self-contained functions meant to introduce you to dtcore.

## Demo catalog

--8<-- "mkdocs/docs/demos/list.md"

## Running demos

Check out the [Quick Start](../guides/quick_start.md) for the easiest path.

For platform details, take a look at [Running Apps](../guides/running_apps.md).

## Conventions used by demos

Demos follow the same conventions as the library:

- Flat namespaces using `dt..._` prefixes
- Explicit allocation and disposal
- Structured error propagation via `dterr_t`
- Use of vtable facades when available
- Warning-clean, portable C
