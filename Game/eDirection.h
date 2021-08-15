#pragma once

enum class eDirection
{
    Horizontal,
    Vertical,
    LeftDiagonal,
    RightDiagonal,

    Count
};

void TranslateDirection(const eDirection direction, int32_t* outDx, int32_t* outDy);