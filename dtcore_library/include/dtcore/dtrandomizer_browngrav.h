/*
 * dtrandomizer_browngrav -- Brownian random walk with gravitational attraction.
 *
 * Generates successive integer values using a Brownian motion model with a
 * configurable restoring force toward an attraction point.  Implements the
 * dtrandomizer, dtobject, and dtpackable facades for polymorphic use and
 * binary serialization of configuration and state.
 *
 * cdox v1.0.2
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <dtcore/dterr.h>
#include <dtcore/dtobject.h>
#include <dtcore/dtpackable.h>

#include <dtcore/dtrandomizer.h>

// Configuration structure
typedef struct dtrandomizer_browngrav_config_t
{
  int32_t attraction_point;
  int32_t attraction_strength;
  int32_t noise_intensity;
  int32_t seed;
} dtrandomizer_browngrav_config_t;

// Object structure
typedef struct dtrandomizer_browngrav_t
{
  DTRANDOMIZER_COMMON_MEMBERS;
  dtrandomizer_browngrav_config_t config;
  int32_t _last;
  bool is_malloced;
} dtrandomizer_browngrav_t;

// API methods
dterr_t*
dtrandomizer_browngrav_create(dtrandomizer_browngrav_t** self);
dterr_t*
dtrandomizer_browngrav_init(dtrandomizer_browngrav_t* self);
dterr_t*
dtrandomizer_browngrav_config(dtrandomizer_browngrav_t* self,
                              dtrandomizer_browngrav_config_t* configuration);
dterr_t*
dtrandomizer_browngrav_register_vtables(void);

// facade implementations
DTRANDOMIZER_DECLARE_API(dtrandomizer_browngrav);
DTOBJECT_DECLARE_API(dtrandomizer_browngrav);
DTPACKABLE_DECLARE_API(dtrandomizer_browngrav);

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtrandomizer_browngrav

Brownian motion with attraction toward a fixed point.

This group of functions provides a stateful random sequence generator for integer-valued signals.
Use it for simulations and effect generation that require bounded but wandering behavior.
It is designed to combine random noise with a restoring force toward a target value.


This class implements these facades:

- [dtrandomizer](dtrandomizer.md) - for generating random sequences.
- [dtpackable](dtpackable.md) - for serializing and deserializing the configuration and state.
- [dtobject](dtobject.md) - for basic object operations like creation, copying, and disposal.

## Mini-guide

- Create, initialize and configure the instance before use.
- Configure behavior by supplying attraction and noise parameters through the config structure.
- Generate successive values by calling the next function with a persistent instance.
- Reset the internal state when changing configuration or reseeding the generator.
- Use the facade interfaces for polymorphic handling and serialization.

## Example

```c
dtrandomizer_browngrav_t* r = NULL;
dtrandomizer_handle h = NULL;
dtrandomizer_browngrav_config_t cfg;
int32_t value;

dtrandomizer_browngrav_create(&r);

cfg.attraction_point = 0;
cfg.attraction_strength = 5;
cfg.noise_intensity = 10;
cfg.seed = 0;

dtrandomizer_browngrav_config(r, &cfg);

// use the randomizer facade handle thereafter
h = (dtrandomizer_handle)r;
dtrandomizer_next(h, &value);

// use the object facade to get a string representation
char buffer[128];
dtobject_to_string((dtobject_handle_t)h, buffer, sizeof(buffer));

dtobject_dispose((dtobject_handle_t)h);
```

## Data structures

### dtrandomizer_browngrav_config_t

Defines configuration parameters that control the randomizer behavior.

Members:

> `int32_t attraction_point` Target value toward which the sequence is pulled.  
> `int32_t attraction_strength` Strength of the restoring force toward the target.  
> `int32_t noise_intensity` Maximum magnitude of random deviation per step.  
> `int32_t seed` Seed value used to initialize the random number generator.  

## Functions

### dtrandomizer_browngrav_create

Create a new instance.  This implies dtrandomizer_browngrav_init().

After create, configuration must be applied before use.

Params:

> `dtrandomizer_browngrav_t** self` Output pointer that receives the allocated instance.  

Return: `dterr_t*` Error object on failure or NULL on success.

### dtrandomizer_browngrav_config

Applies a new configuration and resets internal state.

Params:

> `dtrandomizer_browngrav_t* self` Instance to configure.  
> `dtrandomizer_browngrav_config_t* configuration` Configuration values to apply.  

Return: `dterr_t*` Error object on failure or NULL on success.

### dtrandomizer_browngrav_init

Initializes an instance to a default state.

Params:

> `dtrandomizer_browngrav_t* self` Instance to initialize.  

Return: `dterr_t*` Error object on failure or NULL on success.



<!-- FG_IDC: f60c846c-e5be-415a-8b99-9ac71db79e77 | FG_UTC: 2026-01-19T03:07:44Z FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
