#include <assert.h>

#include "Button.h"
#include "../Util/MouseEvent.h"

Button::Button(DDraw* ddraw, const int x, const int y, const size_t width, const size_t height, const wchar_t* text, const eButtonType type)
    : mDDraw(ddraw)
    , mX(x)
    , mY(y)
    , mWidth(width)
    , mHeight(height)
    , mText(text)
    , mType(type)
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

    HFONT oldFont = (HFONT)SelectObject(hdc, mDefaultFont);

    size_t textLen = lstrlen(mText);
    SIZE textSize;
    GetTextExtentPoint32(hdc, mText, textLen, &textSize);

    int textX = mX + (mWidth / 2) - textSize.cx / 2;
    int textY = mY + (mHeight / 2) - textSize.cy / 2;

    mDDraw->DrawRectangle(mX, mY, mWidth, mHeight, mColor);
    mDDraw->PrintText(hdc, mText, textX, textY, textLen, mTextColor);

    SelectObject(hdc, oldFont);

    mDDraw->EndGDI(hdc);
}

bool Button::IsPressed() const
{
    size_t x = MouseEvent::GetX();
    size_t y = MouseEvent::GetY();

    if ((int)x >= mX && (int)x <= mX + mWidth
        && (int)y >= mY && (int)y <= mY + mHeight)
    {
        return true;
    }
    
    return false;
}