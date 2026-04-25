# dtcore

[![CI](https://github.com/david-erb/dtcore/actions/workflows/ci.yml/badge.svg)](https://github.com/david-erb/dtcore/actions/workflows/ci.yml) [![GitHub tag](https://img.shields.io/github/v/tag/david-erb/dtcore)](https://github.com/david-erb/dtcore/tags)

**Portable C building blocks for real embedded systems.**  
_Current release: v1.0.0_

`dtcore` is a modular C library used as a foundation for portable embedded software across multiple targets.  
It focuses on small, composable modules that implement patterns that recur in real systems: bounded buffers, explicit error propagation, logging, and lightweight interface dispatch.

The core library is platform-agnostic. Where operating system services are required, adapters isolate platform-specific code. In practice, the same modules are built and exercised on **Linux**, **Zephyr**, **ESP-IDF**, and **bare-metal Raspberry Pi Pico**.

This repository reflects the design style I use in production and client embedded systems.

---

## Documentation

The documentation site picks up where this README leaves off:

https://david-erb.github.io/dtcore/

---

## Runnable demos

- **`demo_hello`** — minimal end-to-end example  
- **`demo_basics`** — foundational patterns and utilities  

Each demo is a complete, buildable C program intended to be read and run.  
To get into the details—including embedded source code and design notes—start at the documentation site.

https://david-erb.github.io/dtcore/demos/index.html

---

## Example

```c
#include <stdio.h>

#include <dtcore/dtbuffer.h>
#include <dtcore/dtlog.h>

#define TAG "dtcore_demo_buffer"

// ------------------------------------------------------------------------
// Example: bounded buffer usage with explicit error handling
void
dtcore_demo_buffer(void)
{
    dterr_t* dterr = NULL;
    dtbuffer_t* buffer = NULL;

    // allocate a buffer of 128 bytes
    DTERR_C(dtbuffer_create(&buffer, 128));

    // fill the buffer with some data
    snprintf(buffer->payload, buffer->length, 
        "I'm in the dtcore buffer!  My total size is %" PRId32 " bytes.", 
        buffer->length);

    dtlog_info(TAG, "buffered payload is: %s", (char*)buffer->payload);

cleanup:
    // free the buffer
    dtbuffer_dispose(buffer);

    if (dterr != NULL)
    {
        dtlog_dterr(TAG, dterr);
        dterr->dispose(dterr);
        dterr = NULL;
    }
}
```

---

## Quick verification build (Linux)

A minimal host build to demonstrate that the project compiles and runs.  
Full platform coverage is documented on the documentation site.

```bash
git clone https://github.com/david-erb/dtcore.git
cd dtcore/apps_linux/demo_hello
cmake -S . -B build
cmake --build build
./build/app
```

---

## License

MIT License.