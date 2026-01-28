# Design principles

`dtcore` is guided by a small set of explicit design principles intended to support long-lived, portable embedded software written in C.

## Flat namespaces

All symbols are explicitly prefixed and organized into flat namespaces.  
This avoids implicit coupling and keeps symbol ownership obvious.

## Explicit ownership and lifecycle

Memory allocation, ownership transfer, and disposal are always explicit.  
Modules never hide allocation or rely on global cleanup.

## Structured error propagation

Errors are propagated explicitly through return values and error objects.  
There is no reliance on global state or thread-local error storage.

## Platform isolation

Operating system services (time, logging, threading, I/O) are accessed through narrow adapters.  
The core library remains platform-agnostic.

## Readable, portable C

The code prioritizes clarity over cleverness:

- strict compiler warnings,
- no reliance on undefined behavior,
- patterns intended to be read and maintained by humans.

## Tests as documentation

Unit tests are written to be read as examples.  
They are part of the public API documentation, not just regression guards.