#include <assert.h>
#include <cstdint>

#include "eSpotInfo.h"

bool IsOpen(const eSpotInfo spotInfo)
{
    return (uint32_t)spotInfo % 2 == 0;
}

uint32_t GetSpotInfoHash(const eSpotInfo spotInfo)
{
    assert(static_cast<uint32_t>(spotInfo) <= 10);
    return static_cast<uint32_t>(spotInfo);
}