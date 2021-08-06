#pragma once

#include "eButtonType.h"
#include "../DDraw/DDraw.h"

class Button
{
public:
    Button(DDraw* ddraw, const int x, const int y, const size_t width, const size_t height, const wchar_t* text, const eButtonType type);
    Button(const Button&) = delete;
    ~Button();

    void Draw() const;

    int GetX() const { return mX; }
    void SetX(const int x) { mX = x; }

    int GetY() const { return mY; }
    void SetY(const int y) { mY = y; }

    int GetWidth() const { return mWidth; }
    void SetWidth(const size_t width) { mWidth = width; }

    int GetHeight() const { return mHeight; }
    void SetHeight(const size_t height) { mHeight = height; }

    unsigned int GetColor() const { return mColor; }
    void SetColor(const unsigned int color) { mColor = color; }

    eButtonType GetButtonType() const { return mType; }
    void SetButtonType(const eButtonType type) { mType = type; }

    const wchar_t* GetText() const { return mText; }
    void SetText(const wchar_t* text) { mText = text; }

    unsigned int GetTextColor() const { return mTextColor; }
    void SetTextColor(const unsigned int color) { mTextColor = color; }

    bool IsPressed() const;

private:
    static const unsigned int DEFAULT_COLOR = 0x8c8c8cU;
    static const unsigned int DEFAULT_TEXT_COLOR = 0x000000U;

    DDraw* mDDraw = nullptr;

    int mX = 0;
    int mY = 0;
    size_t mWidth = 0;
    size_t mHeight = 0;
    unsigned int mColor = DEFAULT_COLOR;
    eButtonType mType = eButtonType::None;

    HFONT mDefaultFont = nullptr;
    const wchar_t* mText = L"";
    unsigned int mTextColor = DEFAULT_TEXT_COLOR;
};