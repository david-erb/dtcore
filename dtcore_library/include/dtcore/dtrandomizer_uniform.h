#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <dtcore/dterr.h>
#include <dtcore/dtlcg.h>
#include <dtcore/dtobject.h>
#include <dtcore/dtpackable.h>
#include <dtcore/dtrandomizer.h>

// configuration for this implementation
typedef struct dtrandomizer_uniform_config_t
{
  int32_t home;
  int32_t step;
  int32_t edge;
  int32_t seed;
} dtrandomizer_uniform_config_t;

// the implementation
typedef struct dtrandomizer_uniform_t
{
  DTRANDOMIZER_COMMON_MEMBERS;
  dtrandomizer_uniform_config_t config;
  dtlcg32_t lcg;
  int32_t _last;
  int32_t _is_rand_initialized;
  bool is_malloced;
} dtrandomizer_uniform_t;

typedef struct dtrandomizer_uniform_metrics_t
{
  uint64_t next_metric1;
  uint64_t next_metric2;
} dtrandomizer_uniform_metrics_t;

dterr_t*
dtrandomizer_uniform_create(dtrandomizer_uniform_t** this);

dterr_t*
dtrandomizer_uniform_init(dtrandomizer_uniform_t* this);

dterr_t*
dtrandomizer_uniform_config(dtrandomizer_uniform_t* this,
                            dtrandomizer_uniform_config_t* configuration);
dterr_t*
dtrandomizer_uniform_register_vtables(void);

// runtime usage metrics
dtrandomizer_uniform_metrics_t*
dtrandomizer_uniform_metrics_fetch(void);

// --------------------------------------------------------------------------------------------

// facade implementations
DTRANDOMIZER_DECLARE_API(dtrandomizer_uniform);
DTOBJECT_DECLARE_API(dtrandomizer_uniform);
DTPACKABLE_DECLARE_API(dtrandomizer_uniform);

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtrandomizer_uniform

Uniform step random walk around a bounded home.

This group of functions provides a stateful random sequence generator for integer-valued signals.
Use it where a deterministic model needs to be configured and advanced step-by-step.
The generator produces successive values by applying a uniform random step within configured bounds.

This class implements these facades:

- [dtrandomizer](dtrandomizer.md) - for generating random sequences.
- [dtpackable](dtpackable.md) - for serializing and deserializing the configuration and state.
- [dtobject](dtobject.md) - for basic object operations like creation, copying, and disposal.

## Mini-guide

- Create, initialize and configure the instance before use.
- Configure behavior by supplying home, step, edge, and seed parameters through the config structure.
- Generate successive values by calling the next function with a persistent instance.
- Reset the internal state when changing configuration or reseeding the generator.
- Use the facade interfaces for polymorphic handling and serialization.

## Example

```c
dtrandomizer_uniform_t* r = NULL;
dtrandomizer_uniform_config_t cfg = { .home = 100, .step = 5, .edge = 20, .seed = 0 };
int32_t v = 0;

// create, initialize, and configure the randomizer
dtrandomizer_uniform_create(&r);
dtrandomizer_uniform_config(r, &cfg);

// use the randomizer facade handle thereafter
h = (dtrandomizer_handle)r;
dtrandomizer_next(h, &value);

// use the object facade to get a string representation
char buffer[128];
dtobject_to_string((dtobject_handle_t)h, buffer, sizeof(buffer));

dtobject_dispose((dtobject_handle_t)h);
}
```

## Data structures

### dtrandomizer_uniform_config_t

Defines the configuration used to reset and advance the uniform randomizer.

Members
:

> `int32_t home` Center value used as the reset position.  
> `int32_t step` Maximum absolute delta applied per advance.  
> `int32_t edge` Maximum absolute deviation from `home`.  
> `int32_t seed` Seed value used during reset, with zero selecting time-based seeding.  

## Functions

### dtrandomizer_uniform_create

Create a new instance.  This implies dtrandomizer_uniform_init().

After create, configuration must be applied before use.

Params:

> `dtrandomizer_uniform_t** self` Output pointer that receives the allocated instance.  

Return: `dterr_t*` Error object on failure or NULL on success.

### dtrandomizer_uniform_config

Applies a configuration and resets the internal generator state.

Params
:

> `dtrandomizer_uniform_t* this` Target instance to configure.  
> `dtrandomizer_uniform_config_t* configuration` Configuration values to copy into the instance.  

Return: `dterr_t*` Error object on failure, or `NULL` on success.

### dtrandomizer_uniform_init

Initializes an instance by clearing state and setting the model number.

Params
:

> `dtrandomizer_uniform_t* this` Target instance to initialize.  

Return: `dterr_t*` Error object on failure, or `NULL` on success.

<!-- FG_IDC: 702b4ebc-b3fd-4a88-a3a6-5fe47fad7e66 | FG_UTC: 2026-01-19T03:19:40Z -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
