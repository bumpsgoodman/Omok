#pragma once

enum class eSpotInfo
{
    // ���ǹ��� ��
    None,

    // ����
    Open1,
    Open2,
    Open3,
    Open4,
    Open5,
    Open6,

    // ����
    Closed1,
    Closed2,
    Closed3,
    Closed4,
};

bool IsOpen(const eSpotInfo spotInfo);