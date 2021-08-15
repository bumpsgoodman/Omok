#include "eSpotInfo.h"

bool IsOpen(const eSpotInfo spotInfo)
{
    return (spotInfo >= eSpotInfo::Open1 && spotInfo <= eSpotInfo::Open6);
}