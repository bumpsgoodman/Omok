#include "MouseEvent.h"

namespace MouseEvent
{
    static bool s_bClicked = false;
    static size_t s_ClickedX;
    static size_t s_ClickedY;

    void OnClick(const size_t x, const size_t y)
    {
        s_bClicked = true;
        s_ClickedX = x;
        s_ClickedY = y;
    }

    bool IsClicked()
    {
        return s_bClicked;
    }

    void Release()
    {
        s_bClicked = false;
        s_ClickedX = -1;
        s_ClickedY = -1;
    }

    size_t GetX()
    {
        return s_ClickedX;
    }

    size_t GetY()
    {
        return s_ClickedY;
    }
}