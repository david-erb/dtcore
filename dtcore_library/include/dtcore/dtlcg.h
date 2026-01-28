#pragma once

#include <stdint.h>

typedef struct
{
    uint32_t state;
} dtlcg32_t;

extern void
dtlcg32_seed(dtlcg32_t* lcg, uint32_t seed);

extern uint32_t
dtlcg32_next(dtlcg32_t* lcg);

extern int32_t
dtlcg32_next_range(dtlcg32_t* lcg, int32_t min, int32_t max);

#if MARKDOWN_DOCUMENTATION
// clang-format off
// --8<-- [start:markdown-documentation]

# dtlcg

Simple linear congruential random number generator.

This group of functions provides deterministic pseudo-random number generation for 32-bit values. 
It is intended for lightweight use in tests, simulations, and embedded utilities.
It is designed to be small and predictable.

## Mini-guide

- Initialize the generator state explicitly by calling the seed function before use.
- Generate successive values by calling the next function on the same state instance.
- Derive bounded values by using the range function with an explicit minimum and maximum.
- Maintain one state structure per independent random sequence.
- Avoid using the generator for cryptographic or security-sensitive purposes.

## Example

```c
dtlcg32_t rng;
dtlcg32_seed(&rng, 1234);

uint32_t v1 = dtlcg32_next(&rng);
uint32_t v2 = dtlcg32_next(&rng);

int32_t r = dtlcg32_next_range(&rng, 0, 10);
```

## Data structures

### dtlcg32_t

Holds the internal state for a 32-bit linear congruential generator.

Members:

> `uint32_t state` Internal generator state.  

## Functions

### dtlcg32_next

Advances the generator state and produces the next 32-bit value.

Params:

> `dtlcg32_t* lcg` Pointer to the generator state.  

Return: `uint32_t` The next pseudo-random value.

### dtlcg32_next_range

Advances the generator state and produces a bounded signed integer value.

Params:

> `dtlcg32_t* lcg` Pointer to the generator state.  
> `int32_t min` Lower bound of the output range.  
> `int32_t max` Upper bound of the output range.  

Return: `int32_t` A pseudo-random value in the specified range.

### dtlcg32_seed

Initializes the generator state with a fixed seed value.

Params:

> `dtlcg32_t* lcg` Pointer to the generator state.  
> `uint32_t seed` Initial seed value for the generator.  

Return: `void`  No return value.

<!-- FG_IDC: a4cabb32-61f0-47d5-be27-d63fd184d3bd | FG_UTC: 2026-01-18T09:21:48Z | FG_MAN=yes -->

// --8<-- [end:markdown-documentation]
// clang-format on
#endif
