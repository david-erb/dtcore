#include <stdint.h>

#include <dtcore/dtlcg.h>

// Constants used in glibc or Numerical Recipes
#define LCG_A 1664525
#define LCG_C 1013904223

void
dtlcg32_seed(dtlcg32_t* lcg, uint32_t seed)
{
    lcg->state = seed;
}

uint32_t
dtlcg32_next(dtlcg32_t* lcg)
{
    lcg->state = lcg->state * LCG_A + LCG_C;
    return lcg->state;
}

int32_t
dtlcg32_next_range(dtlcg32_t* lcg, int32_t min, int32_t max)
{
    uint32_t r = dtlcg32_next(lcg);
    return min + (r % (max - min));
}
