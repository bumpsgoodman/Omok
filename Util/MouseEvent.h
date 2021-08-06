#pragma once

namespace MouseEvent
{
    void OnClick(const size_t x, const size_t y);
    bool IsClicked();
    void Release();
    size_t GetX();
    size_t GetY();
}