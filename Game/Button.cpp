#include <assert.h>
#include <cstdint>

#include "Button.h"
#include "../Util/Event.h"

Button::Button(DDraw* ddraw, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height, const eButtonType type, const wchar_t* const text)
    : mDDraw(ddraw)
    , mX(x)
    , mY(y)
    , mWidth(width)
    , mHeight(height)
    , mType(type)
    , mText(text)
{
    assert(mDDraw != nullptr);

    mDefaultFont = CreateFont((int)(mHeight / 3.5f), 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, DEFAULT_QUALITY, FIXED_PITCH, L"Consolas");
}

Button::~Button()
{
    DeleteObject(mDefaultFont);
}

void Button::Draw() const
{
    assert(mDDraw != nullptr);

    HDC hdc;
    mDDraw->BeginGDI(&hdc);

    const HFONT oldFont = (HFONT)SelectObject(hdc, mDefaultFont);

    const uint32_t textLen = lstrlen(mText);
    SIZE textSize;
    GetTextExtentPoint32(hdc, mText, textLen, &textSize);

    const uint32_t textX = mX + (mWidth / 2) - textSize.cx / 2;
    const uint32_t textY = mY + (mHeight / 2) - textSize.cy / 2;

    mDDraw->DrawRectangle(mX, mY, mWidth, mHeight, mColor);
    mDDraw->PrintText(hdc, mText, textX, textY, textLen, mTextColor);

    SelectObject(hdc, oldFont);

    mDDraw->EndGDI(hdc);
}

bool Button::IsPressed() const
{
    const uint32_t x = event::mouse::GetX();
    const uint32_t y = event::mouse::GetY();
    if (x >= mX && x <= mX + mWidth
        && y >= mY && y <= mY + mHeight)
    {
        return true;
    }

    return false;
}