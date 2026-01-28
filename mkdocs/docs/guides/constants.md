# Constants

## Purpose and scope

Several parts of **dtcore** rely on a shared set of numeric and symbolic constants that must remain *bit-for-bit identical* across languages and platforms. These constants are used in binary encoding and decoding paths where even a small mismatch would cause silent corruption, incompatibility, or hard-to-diagnose failures.

The constants are therefore treated as **cross-wire definitions**: a single logical source, rendered verbatim into multiple target languages.

Target languages and environments currently include:

- **C** (Linux, ESP-IDF, nRF5340, Zephyr, Raspberry Pi Pico)
- **C#** (.NET on Windows and Linux)
- **Python** (camera, graphing, imaging and inferencing engines)
- **TypeScript** (Web and tooling)

The same constant set must be usable on desktop systems, embedded MCUs, and web runtimes.

---

## Source of truth

The authoritative source is a YAML file:

```
dtcore/include/dtcore_constants.yaml
```

This file defines all shared constants in a language-neutral, declarative form.  
It is intentionally simple and constrained so it can be safely mapped into multiple syntaxes without interpretation or transformation.

From this YAML file, language-specific constant definitions are generated.

---

## Generated outputs

For C, the generated output is:

```
dtcore/include/dtcore_constants.h
```

This header is **not handwritten**. It is produced mechanically from the YAML file and should be treated as generated code.

Equivalent generated files exist (or are expected to exist) for:

- C#
- Python
- TypeScript

Each generated file expresses the *same constants* using the idioms of the target language, but without changing values, ordering, or semantics.

> **Important:**  
> The generated files are intended to be included directly by consuming code.  
> They should not be edited manually.

---

## Generation process

A Python script performs the YAML-to-language conversion.

- The script reads `dtcore_constants.yaml`
- It emits one output file per target language
- The mapping is direct and deterministic
- No language-specific logic is allowed to alter meaning or numeric values

The generator script itself is **not included in this repository**, by design.  
It is treated as a tooling artifact rather than a runtime dependency.

If you need to regenerate constants for an additional language or environment, the script can be provided on request by contacting the author.

---

## Function groups relying on these constants

Within **dtcore**, the following function groups depend directly on the shared constants:

### `dtkvp`

Used for defining and interpreting **serialization signatures**.  
Constants define identifiers and structural expectations that must match across producers and consumers.

### `dtrandomizer` implementations

Used for **vtable-based model numbering** and selection.  
Constants ensure that model identifiers and randomization behavior are stable and interoperable across builds and languages.

---

## Design rationale

This approach deliberately avoids:

- Duplicating constant definitions per language
- Relying on ad-hoc documentation to keep values in sync
- Introducing runtime negotiation or version probing

Instead, it favors:

- A single, auditable source of truth
- Mechanical, deterministic generation
- Compile-time visibility of mismatches

The result is predictable behavior across heterogeneous systems, consistent with the overall dtcore design goal:

> **Portable C for real systems, shaped by practical use.**
