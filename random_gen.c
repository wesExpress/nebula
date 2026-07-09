#include "random_gen.h"

#include "randommw/randommw.h"

void  random_init()
{
    RanInit("", 0, 0);
}

float random_float()
{
    return DRanU();
}
