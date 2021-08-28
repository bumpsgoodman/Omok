#pragma once

enum class eSpotInfo
{
    None,

    Closed1,
    Closed2,
    Closed3,
    Closed4,

    Open1,
    Open2,
    Open3,
    Open4,
    Open5,
    Open6,
};

eSpotInfo operator-(const eSpotInfo spotInfo, const int32_t level);
eSpotInfo operator+(const eSpotInfo spotInfo, const int32_t level);

bool IsOpen(const eSpotInfo spotInfo);
uint32_t GetSpotInfoHash(const eSpotInfo spotInfo);
eSpotInfo ReverseSpotInfo(const eSpotInfo spotInfo);
eSpotInfo OpenToClosed(const eSpotInfo spotInfo);
eSpotInfo ClosedToOpen(const eSpotInfo spotInfo);
eSpotInfo MaxSpotInfo(const eSpotInfo spotInfo1, const eSpotInfo spotInfo2);
eSpotInfo MinSpotInfo(const eSpotInfo spotInfo1, const eSpotInfo spotInfo2);
eSpotInfo MakeSpotInfo(const uint32_t chainCount, const bool bOpen);