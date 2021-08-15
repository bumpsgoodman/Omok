#include <assert.h>
#include <cstdint>

#include "eDirection.h"

void TranslateDirection(const eDirection direction, int32_t* outDx, int32_t* outDy)
{
    assert(outDx != nullptr);
    assert(outDy != nullptr);

    switch (direction)
    {
    case eDirection::Horizontal:
        *outDx = -1;
        *outDy = 0;
        break;
    case eDirection::Vertical:
        *outDx = 0;
        *outDy = -1;
        break;
    case eDirection::LeftDiagonal:
        *outDx = -1;
        *outDy = -1;
        break;
    case eDirection::RightDiagonal:
        *outDx = 1;
        *outDy = -1;
        break;
    default:
        assert(false);
        break;
    }
}