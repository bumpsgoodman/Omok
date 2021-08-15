#pragma once

#include "eButtonType.h"
#include "../DDraw/DDraw.h"

class Button final
{
public:
    Button(DDraw* ddraw, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height, const eButtonType type, const wchar_t* const text);
    Button(const Button&) = delete;
    ~Button();

    void Draw() const;

    uint32_t GetX() const { return mX; }
    uint32_t GetY() const { return mY; }
    uint32_t GetWidth() const { return mWidth; }
    uint32_t GetHeight() const { return mHeight; }
    eButtonType GetButtonType() const { return mType; }

    uint32_t GetColor() const { return mColor; }
    void SetColor(const uint32_t color) { mColor = color; }

    const wchar_t* const GetText() const { return mText; }
    void SetText(const wchar_t* const text) { mText = text; }

    uint32_t GetTextColor() const { return mTextColor; }
    void SetTextColor(const uint32_t color) { mTextColor = color; }

    bool IsPressed() const;

private:
    static constexpr uint32_t DEFAULT_COLOR = 0x8c8c8cU;
    static constexpr uint32_t DEFAULT_TEXT_COLOR = 0x000000U;

private:
    DDraw* mDDraw = nullptr;

    uint32_t mX;
    uint32_t mY;
    uint32_t mWidth;
    uint32_t mHeight;
    eButtonType mType;
    uint32_t mColor = DEFAULT_COLOR;

    HFONT mDefaultFont;
    const wchar_t* mText;
    uint32_t mTextColor = DEFAULT_TEXT_COLOR;
};