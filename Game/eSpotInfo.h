#pragma once

enum class eSpotInfo
{
    // ���ǹ��� ��
    None,

    Closed1,
    Open1,
    Closed2,
    Open2,
    Closed3,
    Open3,
    Closed4,
    Open4,

    Open5,
    Open6,
};

bool IsOpen(const eSpotInfo spotInfo);
uint32_t GetSpotInfoHash(const eSpotInfo spotInfo);