#include <assert.h>
#include <cstdint>
#include <Windows.h>

#include "eSpotInfo.h"

eSpotInfo operator-(const eSpotInfo spotInfo, const int32_t level)
{
    if (spotInfo == eSpotInfo::None)
    {
        return eSpotInfo::None;
    }

    const int32_t result = (int32_t)spotInfo - level;
    if (IsOpen(spotInfo))
    {
        return (eSpotInfo)(max(result, (int32_t)eSpotInfo::Open1));
    }
    else
    {
        return (eSpotInfo)(max(result, (int32_t)eSpotInfo::Closed1));
    }
}

eSpotInfo operator+(const eSpotInfo spotInfo, const int32_t level)
{
    if (spotInfo == eSpotInfo::None)
    {
        return eSpotInfo::None;
    }

    const int32_t result = (int32_t)spotInfo + level;
    if (IsOpen(spotInfo))
    {
        return (eSpotInfo)(min(result, (int32_t)eSpotInfo::Open6));
    }
    else
    {
        return (eSpotInfo)(min(result, (int32_t)eSpotInfo::Closed4));
    }
}

bool IsOpen(const eSpotInfo spotInfo)
{
    return (spotInfo >= eSpotInfo::Open1 && spotInfo <= eSpotInfo::Open4);
}

uint32_t GetSpotInfoHash(const eSpotInfo spotInfo)
{
    assert(spotInfo <= eSpotInfo::Open6);
    return (uint32_t)spotInfo;
}

eSpotInfo ReverseSpotInfo(const eSpotInfo spotInfo)
{
    if (spotInfo == eSpotInfo::None
        || spotInfo == eSpotInfo::Open5
        || spotInfo == eSpotInfo::Open6)
    {
        return spotInfo;
    }

    if (IsOpen(spotInfo))
    {
        return (eSpotInfo)((uint32_t)spotInfo - 4);
    }
    else
    {
        return (eSpotInfo)((uint32_t)spotInfo + 4);
    }
}

eSpotInfo OpenToClosed(const eSpotInfo spotInfo)
{
    if (IsOpen(spotInfo))
    {
        return (eSpotInfo)((uint32_t)spotInfo - 4);
    }
    else
    {
        return spotInfo;
    }
}

eSpotInfo ClosedToOpen(const eSpotInfo spotInfo)
{
    if (IsOpen(spotInfo))
    {
        return spotInfo;
    }
    else
    {
        if (spotInfo == eSpotInfo::None
            || spotInfo == eSpotInfo::Open5
            || spotInfo == eSpotInfo::Open6)
        {
            return spotInfo;
        }

        return (eSpotInfo)((uint32_t)spotInfo + 4);
    }
}

eSpotInfo MaxSpotInfo(const eSpotInfo spotInfo1, const eSpotInfo spotInfo2)
{
    uint32_t spot1 = (uint32_t)spotInfo1;
    uint32_t spot2 = (uint32_t)spotInfo2;
    bool bOpen1 = false;
    bool bOpen2 = false;

    if (IsOpen(spotInfo1))
    {
        spot1 -= 4;
        bOpen1 = true;
    }

    if (IsOpen(spotInfo2))
    {
        spot2 -= 4;
        bOpen2 = true;
    }

    if (spot1 == spot2)
    {
        return (bOpen1) ? spotInfo1 : spotInfo2;
    }
    else if (spot1 > spot2)
    {
        return spotInfo1;
    }
    else
    {
        return spotInfo2;
    }
}

eSpotInfo MakeSpotInfo(const uint32_t chainCount, const bool bOpen)
{
    if (chainCount == 0)
    {
        return eSpotInfo::None;
    }
    else if (chainCount == 5)
    {
        return eSpotInfo::Open5;
    }
    else if (chainCount >= 6)
    {
        return eSpotInfo::Open6;
    }

    if (bOpen)
    {
        return (eSpotInfo)(chainCount + 4);
    }
    else
    {
        return (eSpotInfo)chainCount;
    }
}